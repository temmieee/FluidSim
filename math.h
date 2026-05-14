#ifndef MATH_H
#define MATH_H

#include <cmath>
#include <array>


using Mat4 = std::array<std::array<float, 4>, 4>;
Mat4 Multiply(const Mat4& a, const Mat4& b);
Mat4 GenerateWorldMatrix(const float position[], const float rotation[], const float scale[]);
#endif