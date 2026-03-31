#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include "KalmanFilter.h"

// ─── CSV Loader ─────────────────────────────────────────────────────

std::vector<double> loadClosePrices(const std::string& filename) {
    std::vector<double> prices;
    std::ifstream file(filename);
    std::string line;

    // Skip header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;

        // CSV columns: Date,Open,High,Low,Close,...
        // Skip Date
        std::getline(ss, token, ',');

        // Skip Open, High, Low (columns 1-3)
        for (int i = 0; i < 3; ++i)
            std::getline(ss, token, ',');

        // Read Close (column 4)
        std::getline(ss, token, ',');
        if (!token.empty()) {
            try {
                prices.push_back(std::stod(token));
            } catch (...) {
                continue;
            }
        }
    }
    return prices;
}

// ─── Baseline Filters ───────────────────────────────────────────────

std::vector<double> computeSMA(const std::vector<double>& data, int window) {
    std::vector<double> sma(data.size(), 0.0);
    for (size_t i = 0; i < data.size(); ++i) {
        if (i < static_cast<size_t>(window - 1)) {
            sma[i] = data[i];
        } else {
            double sum = 0.0;
            for (int j = 0; j < window; ++j)
                sum += data[i - j];
            sma[i] = sum / window;
        }
    }
    return sma;
}

std::vector<double> computeEMA(const std::vector<double>& data, int period) {
    std::vector<double> ema(data.size(), 0.0);
    double alpha = 2.0 / (period + 1);
    ema[0] = data[0];
    for (size_t i = 1; i < data.size(); ++i)
        ema[i] = alpha * data[i] + (1.0 - alpha) * ema[i - 1];
    return ema;
}

// ─── Metrics ────────────────────────────────────────────────────────

// Variance of tick-to-tick returns (measures smoothness)
double returnVariance(const std::vector<double>& prices, size_t start = 0) {
    std::vector<double> returns;
    for (size_t i = std::max(start, (size_t)1); i < prices.size(); ++i)
        returns.push_back(prices[i] - prices[i - 1]);

    double mean = 0.0;
    for (double r : returns) mean += r;
    mean /= returns.size();

    double var = 0.0;
    for (double r : returns) var += (r - mean) * (r - mean);
    return var / returns.size();
}

// Mean absolute tracking error (how close filter stays to raw price)
double meanAbsError(const std::vector<double>& filtered, const std::vector<double>& raw, size_t start = 0) {
    double sum = 0.0;
    size_t count = 0;
    for (size_t i = start; i < filtered.size() && i < raw.size(); ++i) {
        sum += std::abs(filtered[i] - raw[i]);
        ++count;
    }
    return count > 0 ? sum / count : 0.0;
}

// Max deviation from raw price
double maxDeviation(const std::vector<double>& filtered, const std::vector<double>& raw, size_t start = 0) {
    double maxDev = 0.0;
    for (size_t i = start; i < filtered.size() && i < raw.size(); ++i) {
        double dev = std::abs(filtered[i] - raw[i]);
        if (dev > maxDev) maxDev = dev;
    }
    return maxDev;
}

// ─── Main ───────────────────────────────────────────────────────────

int main() {
    // Load real market data
    std::vector<double> prices = loadClosePrices("market_data.csv");

    if (prices.size() < 50) {
        std::cerr << "Error: insufficient data (need 50+ prices, got "
                  << prices.size() << ")\n";
        return 1;
    }

    std::cout << "Loaded " << prices.size() << " daily close prices\n";
    std::cout << "Price range: " << *std::min_element(prices.begin(), prices.end())
              << " — " << *std::max_element(prices.begin(), prices.end()) << "\n\n";

    // ─── Estimate noise parameters from data ───
    // Process noise Q: variance of price changes (how much true price drifts)
    // Measurement noise R: we assume observed close has some microstructure noise
    double diffVar = returnVariance(prices);
    double Q = diffVar * 0.5;   // Half of observed variance attributed to true movement
    double R = diffVar * 0.5;   // Half attributed to observation noise

    KalmanFilter kf(Q, R, prices[0]);
    std::vector<double> kalman_filtered(prices.size());
    kalman_filtered[0] = prices[0];
    for (size_t i = 1; i < prices.size(); ++i)
        kalman_filtered[i] = kf.update(prices[i]);

    // ─── Run Baselines ───
    const int sma_window = 20;
    const int ema_period = 20;
    std::vector<double> sma = computeSMA(prices, sma_window);
    std::vector<double> ema = computeEMA(prices, ema_period);

    // ─── Benchmark ───
    size_t warmup = static_cast<size_t>(sma_window);

    double var_raw    = returnVariance(prices, warmup);
    double var_kalman = returnVariance(kalman_filtered, warmup);
    double var_sma    = returnVariance(sma, warmup);
    double var_ema    = returnVariance(ema, warmup);

    double mae_kalman = meanAbsError(kalman_filtered, prices, warmup);
    double mae_sma    = meanAbsError(sma, prices, warmup);
    double mae_ema    = meanAbsError(ema, prices, warmup);

    double maxd_kalman = maxDeviation(kalman_filtered, prices, warmup);
    double maxd_sma    = maxDeviation(sma, prices, warmup);
    double maxd_ema    = maxDeviation(ema, prices, warmup);

    double noise_red_kalman = (1.0 - var_kalman / var_raw) * 100.0;
    double noise_red_sma    = (1.0 - var_sma / var_raw) * 100.0;
    double noise_red_ema    = (1.0 - var_ema / var_raw) * 100.0;

    // Filtering efficiency: noise reduction per unit of tracking error
    // Higher = better trade-off between smoothness and responsiveness
    double eff_kalman = noise_red_kalman / mae_kalman;
    double eff_sma    = noise_red_sma / mae_sma;
    double eff_ema    = noise_red_ema / mae_ema;

    // ─── Output ───
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "============================================================\n";
    std::cout << "  Kalman Filter Benchmark — AAPL Daily Close Prices\n";
    std::cout << "============================================================\n";
    std::cout << "  Data: " << prices.size() << " trading days"
              << "  |  Q: " << Q << "  |  R: " << R << "\n\n";

    std::cout << "  Method           Noise Red.   MAE ($)   Max Dev ($)   Efficiency\n";
    std::cout << "  ─────────────    ──────────   ───────   ───────────   ──────────\n";
    std::cout << "  Raw              baseline     —         —             —\n";
    std::cout << "  SMA(" << sma_window << ")          "
              << std::setw(6) << noise_red_sma << "%    "
              << std::setw(7) << mae_sma << "   "
              << std::setw(11) << maxd_sma << "   "
              << std::setw(10) << eff_sma << "\n";
    std::cout << "  EMA(" << ema_period << ")          "
              << std::setw(6) << noise_red_ema << "%    "
              << std::setw(7) << mae_ema << "   "
              << std::setw(11) << maxd_ema << "   "
              << std::setw(10) << eff_ema << "\n";
    std::cout << "  Kalman          "
              << std::setw(6) << noise_red_kalman << "%    "
              << std::setw(7) << mae_kalman << "   "
              << std::setw(11) << maxd_kalman << "   "
              << std::setw(10) << eff_kalman << "\n";

    std::cout << "\n  Kalman tracking: " << std::setprecision(1)
              << mae_kalman / mae_sma * 100.0 << "% of SMA error, "
              << mae_kalman / mae_ema * 100.0 << "% of EMA error\n";
    std::cout << "  Kalman efficiency: " << std::setprecision(1)
              << eff_kalman / eff_sma << "x SMA, "
              << eff_kalman / eff_ema << "x EMA\n";
    std::cout << "============================================================\n";

    // ─── Export CSV ───
    std::ofstream csv("benchmark_results.csv");
    csv << "tick,close,kalman,sma_" << sma_window << ",ema_" << ema_period << "\n";
    for (size_t i = 0; i < prices.size(); ++i) {
        csv << i << "," << prices[i] << "," << kalman_filtered[i]
            << "," << sma[i] << "," << ema[i] << "\n";
    }
    csv.close();
    std::cout << "\n  Results exported to benchmark_results.csv\n";

    return 0;
}
