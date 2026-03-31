
#ifndef KALMANFILTER_H
#define KALMANFILTER_H

class KalmanFilter {
private:
    double Q;      // Process noise covariance
    double R;      // Measurement noise covariance
    double x_hat;  // State estimate
    double P;      // Estimate covariance

public:
    // Q = process noise covariance (how much true value drifts per tick)
    // R = measurement noise covariance (how noisy observations are)
    // initial_estimate = first guess of the latent state
    KalmanFilter(double process_noise, double measurement_noise, double initial_estimate);

    // Predict step: propagate state and covariance forward
    void predict();

    // Update step: incorporate new measurement, return filtered estimate
    double update(double measurement);

    double getState() const;
    double getCovariance() const;
};

#endif // KALMANFILTER_H
