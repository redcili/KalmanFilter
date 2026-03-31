# Kalman Filter for Market Price Smoothing

## Overview
A C++ implementation of a Kalman filter for real-time market price signal extraction, benchmarked against SMA and EMA baselines on real equity data.

Financial markets are full of noisy data, each price tick isn't always meaningful. The Kalman filter models the market price as a **latent state variable** with Gaussian process and observation noise, running recursive predict-update steps to extract a cleaner signal from raw prices.

Unlike simple moving averages, the Kalman filter **adapts its gain** based on the estimated noise structure of the data, achieving a superior trade-off between smoothness and tracking accuracy.

## Benchmark Results. AAPL Daily Close (506 trading days)

| Method   | Noise Reduction | MAE ($) | Max Deviation ($) | Efficiency |
|----------|----------------:|--------:|-------------------:|-----------:|
| SMA(20)  |         95.29%  |   3.24  |             12.84  |      29.43 |
| EMA(20)  |         95.25%  |   2.73  |             12.28  |      34.90 |
| **Kalman** | **55.35%**    | **0.51**|           **3.06** | **109.23** |

**Key findings:**
- **3.7x higher filtering efficiency** than SMA(20), **3.1x** than EMA(20)
- **84% lower tracking error** (MAE) than SMA, keeping the filtered signal within $3.06 of the true price
- Noise parameters (Q, R) estimated directly from observed return variance — no manual tuning required

> Efficiency = noise reduction per unit of tracking error. A filter that smooths aggressively but deviates $12 from the actual price is not useful for live trading.

## How It Works

The Kalman filter maintains two quantities at each tick:
1. **State estimate** (`x_hat`): current best guess of the true price
2. **Estimate covariance** (`P`): uncertainty in that guess

Each new observation triggers:
- **Predict**: covariance grows by process noise Q (price can drift)
- **Update**: Kalman gain K balances prior vs observation, state and covariance are corrected

```
K = P / (P + R)
x_hat = x_hat + K * (measurement - x_hat)
P = (1 - K) * P
```

## Project Structure
```
KalmanFilter.h       # Filter class declaration
KalmanFilter.cpp     # Predict-update implementation
main.cpp             # Benchmark harness: CSV loader, SMA/EMA baselines, metrics
market_data.csv      # AAPL daily close prices (506 days)
Makefile             # Build system
Kalman_Filter.pdf    # Mathematical derivation
```

## Build and Run

```bash
make          # compile
./kalman      # run benchmark
make clean    # remove build artifacts
```

Output includes a benchmark table and exports `benchmark_results.csv` with all filtered series for further analysis.

## Mathematical Details
See `Kalman_Filter.pdf` for the full derivation of the predict-update equations and noise model.
