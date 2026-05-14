#ifndef MATH_H
#define MATH_H

#include <cmath>
#include <array>

using Mat4 = std::array<std::array<float, 4>, 4>;

Mat4 Multiply(const Mat4& a, const Mat4& b) {
    Mat4 result{};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i][j] = 0.0f;
            for (int k = 0; k < 4; k++) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    return result;
}

Mat4 GenerateWorldMatrix(const float position[3],
    const float rotation[3], // Euler angles: pitch, yaw, roll (radians)
    const float scale[3]) {
    // Scale
    Mat4 S = { {
        {scale[0], 0, 0, 0},
        {0, scale[1], 0, 0},
        {0, 0, scale[2], 0},
        {0, 0, 0, 1}
    } };

    // Rotation X
    float cx = cos(rotation[0]), sx = sin(rotation[0]);
    Mat4 Rx = { {
        {1, 0, 0, 0},
        {0, cx, -sx, 0},
        {0, sx, cx, 0},
        {0, 0, 0, 1}
    } };

    // Rotation Y
    float cy = cos(rotation[1]), sy = sin(rotation[1]);
    Mat4 Ry = { {
        {cy, 0, sy, 0},
        {0, 1, 0, 0},
        {-sy, 0, cy, 0},
        {0, 0, 0, 1}
    } };

    // Rotation Z
    float cz = cos(rotation[2]), sz = sin(rotation[2]);
    Mat4 Rz = { {
        {cz, -sz, 0, 0},
        {sz, cz, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    } };

    // Combined rotation (Z * Y * X)
    Mat4 R = Multiply(Rz, Multiply(Ry, Rx));

    // Translation
    Mat4 T = { {
        {1, 0, 0, position[0]},
        {0, 1, 0, position[1]},
        {0, 0, 1, position[2]},
        {0, 0, 0, 1}
    } };

    // Final matrix: T * R * S
    return Multiply(T, Multiply(R, S));
}
#endif