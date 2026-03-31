#include "KalmanFilter.h"

KalmanFilter::KalmanFilter(double process_noise, double measurement_noise, double initial_estimate)
    : Q(process_noise), R(measurement_noise), x_hat(initial_estimate), P(1.0) {}

void KalmanFilter::predict() {
    // State transition: random walk model (x_hat unchanged)
    // Covariance grows by process noise each tick
    P += Q;
}

double KalmanFilter::update(double measurement) {
    // Predict step
    predict();

    // Kalman gain: balances trust between model and observation
    double K = P / (P + R);

    // State update: weighted combination of prior and measurement
    x_hat = x_hat + K * (measurement - x_hat);

    // Covariance update: certainty improves after observation
    P = (1.0 - K) * P;

    return x_hat;
}

double KalmanFilter::getState() const { return x_hat; }
double KalmanFilter::getCovariance() const { return P; }
