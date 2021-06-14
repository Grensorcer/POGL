#pragma once

#include <iostream>

namespace mygl
{
    class matrix4
    {
    public:
        matrix4();
        matrix4(const float (&arr)[16]);
        static matrix4 identity();

        const float &at(short i, short j) const;
        const float *raw() const;
        matrix4 transpose() const;

        matrix4 &operator*=(const matrix4 &rhs);
        matrix4 &operator+=(const matrix4 &rhs);
        matrix4 &operator-=(const matrix4 &rhs);

        friend void translate(matrix4 &mat, const float &x, const float &y,
                              const float &z);
        friend void frustum(matrix4 &mat, const float &left, const float &right,
                            const float &bottom, const float &top,
                            const float &z_near, const float &z_far);
        friend void perspective(matrix4 &mat, const float &fov,
                                const float &aspect, const float &near,
                                const float &far);
        friend void look_at(matrix4 &mat, const float &eyeX, const float &eyeY,
                            const float &eyeZ, const float &centerX,
                            const float &centerY, const float &centerZ,
                            float upX, float upY, float upZ);
        friend std::ostream &operator<<(std::ostream &out, const matrix4 &m);

    private:
        float arr_[16] = { 0 };
    };

} // namespace mygl
