#pragma once

#include <array>
#include <math.h>


namespace math{

//===============================
// Vector3
//===============================

struct Vector3 {
    std::array<double, 3> data;

    double& x();
    const double& x() const;
    
    double& y();
    const double& y() const;
    
    double& z();
    const double& z() const;

    double& operator[](std::size_t idx);
    const double& operator[](std::size_t idx) const;

    Vector3 operator+(const Vector3& other) const;
};

//===============================
// Matrix3
//===============================

struct Matrix3 {
    std::array<Vector3, 3> data;

    Vector3& operator[](std::size_t idx);

    const Vector3& operator[](std::size_t idx) const;

    // Matrix multiplication: Matrix3 * Matrix3 -> Matrix3
    Matrix3 operator*(const Matrix3& other) const;

    // Matrix-vector multiplication: Matrix3 * Vector3 -> Vector3
    Vector3 operator*(const Vector3& vec) const;
};

//===============================
// MiniMath (helper functions)
//===============================

class MiniMath{
public:
    static const Matrix3 rotation_matrix_from_euler_degrees(const Vector3& rotation_deg);
};

}