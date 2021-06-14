#include "matrix4.hh"

#include <memory>
#include <stdexcept>
#include <cmath>

#include "vec4.hh"

namespace mygl
{
    matrix4::matrix4()
        : arr_{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    {}

    matrix4::matrix4(const float (&arr)[16])
    {
        std::copy(arr, arr + 16, arr_);
    }

    matrix4 matrix4::identity()
    {
        const float arr[16] = {
            1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1
        };

        return matrix4(arr);
    }

    const float &matrix4::at(short i, short j) const
    {
        if (i < 0 || j < 0 || i > 15 || j > 15)
            throw std::out_of_range("indices out of range");
        return this->arr_[4 * i + j];
    }

    const float *matrix4::raw() const
    {
        return arr_;
    }

    matrix4 matrix4::transpose() const
    {
        float arr[16] = { 0 };
        for (size_t i = 0; i < 4; ++i)
            for (size_t j = 0; j < 4; ++j)
                arr[j * 4 + i] = arr_[i * 4 + j];
        return matrix4(arr);
    }

    matrix4 &matrix4::operator*=(const matrix4 &rhs)
    {
        float arr[16];
        std::copy(this->arr_, this->arr_ + 16, arr);
        auto mult_sums = [&](short i, short j) {
            return arr[4 * i] * rhs.arr_[j] + arr[4 * i + 1] * rhs.arr_[j + 4]
                + arr[4 * i + 2] * rhs.arr_[j + 8]
                + arr[4 * i + 3] * rhs.arr_[j + 12];
        };

        for (short i = 0; i < 16; ++i)
            arr_[i] = mult_sums(i / 4, i % 4);

        return *this;
    }

    matrix4 &matrix4::operator+=(const matrix4 &rhs)
    {
        for (short i = 0; i < 16; ++i)
            arr_[i] += rhs.arr_[i];

        return *this;
    }

    matrix4 &matrix4::operator-=(const matrix4 &rhs)
    {
        for (short i = 0; i < 16; ++i)
            arr_[i] -= rhs.arr_[i];

        return *this;
    }

    std::ostream &operator<<(std::ostream &out, const matrix4 &m)
    {
        for (short i = 0; i < 4; ++i)
        {
            for (short j = 0; j < 4; ++j)
                out << m.arr_[i * 4 + j] << " | ";
            out << '\n';
        }

        return out << '\n';
    }

    void translate(matrix4 &mat, const float &x, const float &y, const float &z)
    {
        const auto M =
            matrix4({ 1, 0, 0, x, 0, 1, 0, y, 0, 0, 1, z, 0, 0, 0, 1 });
        mat *= M;
    }

    void frustum(matrix4 &mat, const float &left, const float &right,
                 const float &bottom, const float &top, const float &z_near,
                 const float &z_far)
    {
        const auto mul = matrix4(
            { 2 * z_near / (right - left), 0, (right + left) / (right - left),
              0, 0, 2 * z_near / (top - bottom),
              (top + bottom) / (top - bottom), 0, 0, 0,
              -(z_far + z_near) / (z_far - z_near),
              -2 * z_far * z_near / (z_far - z_near), 0, 0, -1, 0 });

        mat *= mul;
    }

    void perspective(matrix4 &mat, const float &fov, const float &aspect,
                     const float &near, const float &far)
    {
        const auto top = near * std::tan(fov * M_PI / 360);
        const auto right = top * aspect;
        frustum(mat, -right, right, -top, top, near, far);
    }

    void look_at(matrix4 &mat, const float &eyeX, const float &eyeY,
                 const float &eyeZ, const float &centerX, const float &centerY,
                 const float &centerZ, float upX, float upY, float upZ)
    {
        const auto F =
            vec4({ centerX - eyeX, centerY - eyeY, centerZ - eyeZ, 0 })
                .normalize();
        const auto UP = vec4({ upX, upY, upZ, 0 }).normalize();
        auto s = cross3(F, UP);

        const auto s0 = s.at(0);
        const auto s1 = s.at(1);
        const auto s2 = s.at(2);
        s.normalize();
        const auto u = cross3(s, F);

        const auto M = matrix4({ s0, s1, s2, 0, u.at(0), u.at(1), u.at(2), 0,
                                 -F.at(0), -F.at(1), -F.at(2), 0, 0, 0, 0, 1 });

        mat *= M;
        translate(mat, -eyeX, -eyeY, -eyeZ);
    }
} // namespace mygl