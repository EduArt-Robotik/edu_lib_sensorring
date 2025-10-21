#pragma once

#include <array>
#include <cmath>

namespace eduart {

namespace math {

//===============================
// Vector3
//===============================

struct Vector3 {
  std::array<float, 3> data;

  float &x ();
  const float &x () const;

  float &y ();
  const float &y () const;

  float &z ();
  const float &z () const;

  float &operator[] (std::size_t idx);
  const float &operator[] (std::size_t idx) const;

  Vector3 operator+ (const Vector3 &other) const;
};

//===============================
// Matrix3
//===============================

struct Matrix3 {
  std::array<Vector3, 3> data;

  Vector3 &operator[] (std::size_t idx);

  const Vector3 &operator[] (std::size_t idx) const;

  // Matrix multiplication: Matrix3 * Matrix3 -> Matrix3
  Matrix3 operator* (const Matrix3 &other) const;

  // Matrix-vector multiplication: Matrix3 * Vector3 -> Vector3
  Vector3 operator* (const Vector3 &vec) const;
};

//===============================
// MiniMath (helper functions)
//===============================

class MiniMath {
public:
  static const Matrix3 rotation_matrix_from_euler_degrees (const Vector3 &rotation_deg);
};

}

}