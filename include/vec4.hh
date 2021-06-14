#pragma once

namespace mygl
{
    class vec4
    {
    public:
        vec4();
        vec4(const float (&arr)[4]);

        float norm();
        vec4 &normalize();

        float &at(short i);
        const float &at(short i) const;
        vec4 &operator*=(float f);

        friend float operator*(const vec4 &lhs, const vec4 &rhs);
        friend vec4 cross3(const vec4 &lhs, const vec4 &rhs);

    private:
        float arr_[4] = { 0 };
    };
} // namespace mygl