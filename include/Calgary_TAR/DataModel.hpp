#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <Eigen/Dense>

namespace TARapp {

struct TorquePoint {
    float angle = 0.0f;     // Independent variable x
    float rest_v = 0.0f;    // Passive torque / Rest Voltage
    float peak_v = 0.0f;    // Total peak torque / Peak Voltage

    // Derived values
    float activeTorque() const { return peak_v - rest_v; }
};

struct PolyFitResult {
    int degree = 3;
    std::vector<double> coeffs; // ascending powers: c0 + c1*x + c2*x^2 + ...
    double r_squared = 0.0;
    double rmse = 0.0;
    double max_curve_val = 1.0; // max value of P(x) over range
    double min_angle = 0.0;
    double max_angle = 0.0;
    bool valid = false;

    double evaluate(double x) const {
        if (coeffs.empty()) return 0.0;
        double val = 0.0;
        double x_pow = 1.0;
        for (double c : coeffs) {
            val += c * x_pow;
            x_pow *= x;
        }
        return val;
    }

    double evaluateNormalized(double x) const {
        if (std::abs(max_curve_val) <= 1e-9) return 0.0;
        return evaluate(x) / max_curve_val;
    }
};

struct QueryResult {
    float angle = 0.0f;
    float norm_torque = 0.0f;       // P(x) / max_curve_val
    float raw_active_torque = 0.0f; // P(x) * max_raw_active
    float raw_passive_torque = 0.0f;// interpolated from rest_v
    float raw_peak_torque = 0.0f;   // passive + active
};

class DataModel {
public:
    static float getMaxRawActive(const std::vector<TorquePoint>& points) {
        if (points.empty()) return 1.0f;
        float max_raw = points[0].activeTorque();
        for (const auto& pt : points) {
            if (std::abs(pt.activeTorque()) > std::abs(max_raw)) {
                max_raw = pt.activeTorque();
            }
        }
        if (std::abs(max_raw) < 1e-9f) {
            return 1.0f;
        }
        return max_raw;
    }

    static PolyFitResult fitPolynomial(const std::vector<TorquePoint>& points, int degree) {
        PolyFitResult res;
        res.degree = degree;
        res.valid = false;

        if (points.size() < static_cast<size_t>(degree + 1)) {
            return res;
        }

        // Find angle range and max active torque magnitude
        float min_a = points[0].angle;
        float max_a = points[0].angle;

        for (const auto& pt : points) {
            if (pt.angle < min_a) min_a = pt.angle;
            if (pt.angle > max_a) max_a = pt.angle;
        }

        res.min_angle = min_a;
        res.max_angle = max_a;

        float max_raw_active = getMaxRawActive(points);

        // Prepare normalized targets (matching MATLAB: activeT ./ max(activeT))
        size_t N = points.size();
        Eigen::MatrixXd X(N, degree + 1);
        Eigen::VectorXd Y(N);

        for (size_t i = 0; i < N; ++i) {
            double norm_y = static_cast<double>(points[i].activeTorque()) / max_raw_active;
            Y(i) = norm_y;

            double x_val = points[i].angle;
            double x_pow = 1.0;
            for (int col = 0; col <= degree; ++col) {
                X(i, col) = x_pow;
                x_pow *= x_val;
            }
        }

        // Solve least squares: X * coeffs = Y
        Eigen::VectorXd coeffs = X.colPivHouseholderQr().solve(Y);
        res.coeffs.resize(degree + 1);
        for (int i = 0; i <= degree; ++i) {
            res.coeffs[i] = coeffs(i);
        }

        // Compute max value of fitted curve over extended sampling range [floor(min/5)*5 .. ceil(max/5)*5]
        double start_x = std::floor(min_a / 5.0) * 5.0;
        double end_x = std::ceil(max_a / 5.0) * 5.0;
        if (end_x <= start_x) end_x = start_x + 10.0;

        int num_samples = 200;
        double step = (end_x - start_x) / (num_samples - 1);
        double max_pos_val = -1e9;
        double max_abs_val = 0.0;
        double max_abs_signed = 1.0;

        for (int i = 0; i < num_samples; ++i) {
            double x = start_x + i * step;
            double val = res.evaluate(x);
            if (val > max_pos_val) max_pos_val = val;
            if (std::abs(val) > max_abs_val) {
                max_abs_val = std::abs(val);
                max_abs_signed = val;
            }
        }
        double peak_val = (max_pos_val > 1e-9) ? max_pos_val : max_abs_signed;
        if (std::abs(peak_val) <= 1e-9) peak_val = 1.0;
        res.max_curve_val = peak_val;

        // Compute R^2 and RMSE
        double y_sum = Y.sum();
        double y_mean = y_sum / N;
        double ss_tot = 0.0;
        double ss_res = 0.0;

        for (size_t i = 0; i < N; ++i) {
            double y_actual = Y(i);
            double y_pred = res.evaluate(points[i].angle);
            ss_tot += (y_actual - y_mean) * (y_actual - y_mean);
            ss_res += (y_actual - y_pred) * (y_actual - y_pred);
        }

        res.r_squared = (ss_tot > 1e-9) ? (1.0 - (ss_res / ss_tot)) : 1.0;
        res.rmse = std::sqrt(ss_res / N);
        res.valid = true;

        return res;
    }

    // Solve P(x) - target_val = 0 for x in [min_x, max_x]
    static std::vector<double> findRoots(const PolyFitResult& fit, double target_val_norm) {
        std::vector<double> valid_roots;
        if (!fit.valid || fit.coeffs.empty()) return valid_roots;

        // Target value in raw polynomial scale (matching MATLAB line 60: y_target = target_pct * max_val)
        double y_target = target_val_norm * fit.max_curve_val;

        // Shift polynomial: P(x) - y_target = 0
        std::vector<double> c = fit.coeffs;
        c[0] -= y_target;

        int n = static_cast<int>(c.size()) - 1;
        // Strip leading zeros if highest coefficients are 0
        while (n > 0 && std::abs(c[n]) < 1e-12) {
            n--;
        }

        if (n <= 0) return valid_roots;

        // Form Companion Matrix for polynomial c0 + c1*x + ... + cn*x^n = 0
        double cn = c[n];
        Eigen::MatrixXd C = Eigen::MatrixXd::Zero(n, n);
        for (int i = 0; i < n - 1; ++i) {
            C(i + 1, i) = 1.0;
        }
        for (int i = 0; i < n; ++i) {
            C(i, n - 1) = -c[i] / cn;
        }

        Eigen::EigenSolver<Eigen::MatrixXd> solver(C);
        auto eigenvalues = solver.eigenvalues();

        double margin = 0.5; // slight tolerance around angle range
        for (int i = 0; i < eigenvalues.size(); ++i) {
            if (std::abs(eigenvalues(i).imag()) < 1e-4) {
                double root = eigenvalues(i).real();
                if (root >= fit.min_angle - margin && root <= fit.max_angle + margin) {
                    valid_roots.push_back(root);
                }
            }
        }

        std::sort(valid_roots.begin(), valid_roots.end());
        // Remove duplicates within tolerance
        valid_roots.erase(std::unique(valid_roots.begin(), valid_roots.end(), [](double a, double b) {
            return std::abs(a - b) < 1e-3;
        }), valid_roots.end());

        return valid_roots;
    }

    // Interpolate raw passive voltage (Rest_V) at arbitrary angle
    static float interpolatePassive(const std::vector<TorquePoint>& points, float target_angle) {
        if (points.empty()) return 0.0f;
        if (points.size() == 1) return points[0].rest_v;

        // Create sorted copy
        std::vector<TorquePoint> sorted = points;
        std::sort(sorted.begin(), sorted.end(), [](const TorquePoint& a, const TorquePoint& b) {
            return a.angle < b.angle;
        });

        if (target_angle <= sorted.front().angle) return sorted.front().rest_v;
        if (target_angle >= sorted.back().angle) return sorted.back().rest_v;

        for (size_t i = 0; i < sorted.size() - 1; ++i) {
            if (target_angle >= sorted[i].angle && target_angle <= sorted[i + 1].angle) {
                float dx = sorted[i + 1].angle - sorted[i].angle;
                if (dx <= 1e-6f) return sorted[i].rest_v;
                float t = (target_angle - sorted[i].angle) / dx;
                return sorted[i].rest_v + t * (sorted[i + 1].rest_v - sorted[i].rest_v);
            }
        }
        return sorted.back().rest_v;
    }

    static std::vector<TorquePoint> getSampleData() {
        return {
            { 10.0f, 0.45f, 1.25f },
            { 20.0f, 0.50f, 1.85f },
            { 30.0f, 0.60f, 2.60f },
            { 40.0f, 0.75f, 3.45f },
            { 50.0f, 0.95f, 4.10f },
            { 60.0f, 1.20f, 4.40f },
            { 70.0f, 1.55f, 4.25f },
            { 80.0f, 2.00f, 3.80f },
            { 90.0f, 2.50f, 3.10f }
        };
    }
};

} // namespace archi
