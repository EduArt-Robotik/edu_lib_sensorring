#pragma once

#include <array>
#include <math.h>


namespace mm{ // MiniMath


struct Vector3 {
    std::array<double, 3> data;

    double& x() { return data[0]; }
    const double& x() const { return data[0]; }
    
    double& y() { return data[1]; }
    const double& y() const { return data[1]; }
    
    double& z() { return data[2]; }
    const double& z() const { return data[2]; }

    double& operator[](std::size_t idx) { return data[idx]; }
    const double& operator[](std::size_t idx) const { return data[idx]; }

    Vector3 operator+(const Vector3& other) const {
        Vector3 result{};
        for (int i = 0; i < 3; ++i) {
            result[i] = data[i] + other[i];
        }
        return result;
    }
};

struct Matrix3 {
    std::array<Vector3, 3> data;

    Vector3& operator[](std::size_t idx) {
        return data[idx];
    }

    const Vector3& operator[](std::size_t idx) const {
        return data[idx];
    }

    // Matrix multiplication: Matrix3 * Matrix3 -> Matrix3
    Matrix3 operator*(const Matrix3& other) const {
        Matrix3 result{};
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result[i][j] = 0.0f;
                for (int k = 0; k < 3; ++k) {
                    result[i][j] += data[i][k] * other[k][j];
                }
            }
        }
        return result;
    }

    // Matrix-vector multiplication: Matrix3 * Vector3 -> Vector3
    Vector3 operator*(const Vector3& vec) const {
        Vector3 result{};
        for (int i = 0; i < 3; ++i) {
            result[i] = 0.0f;
            for (int j = 0; j < 3; ++j) {
                result[i] += data[i][j] * vec[j];
            }
        }
        return result;
    }
};

class MiniMath{
public:
    static const Matrix3 rotation_matrix_from_euler_degrees(const Vector3& rotation_deg){
        double x_rad = rotation_deg[0] * M_PI / 180.0F;
        double y_rad = rotation_deg[1] * M_PI / 180.0F;
        double z_rad = rotation_deg[2] * M_PI / 180.0F;
        
        Matrix3 r_x = {{{
                        {1, 0, 0},
                        {0, std::cos(x_rad), -std::sin(x_rad)},
                        {0, std::sin(x_rad),  std::cos(x_rad)}}}};
        
        Matrix3 r_y = {{{
                        { std::cos(y_rad), 0, std::sin(y_rad)},
                        { 0, 1, 0},
                        {-std::sin(y_rad), 0, std::cos(y_rad)}}}};
        
        Matrix3 r_z = {{{
                        {std::cos(z_rad), -std::sin(z_rad), 0},
                        {std::sin(z_rad),  std::cos(z_rad), 0},
                        {0, 0, 1}}}};
        
        Matrix3 r = r_z * r_y * r_x;
        
        return r;
    }
};

} // Namespace mm