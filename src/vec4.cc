#include "vec4.hh"

#include <memory>
#include <stdexcept>

namespace mygl
{
    vec4::vec4(const float (&arr)[4])
    {
        std::copy(arr, arr + 4, arr_);
    }

    const float &vec4::at(short i) const
    {
        return arr_[i];
    }
    float &vec4::at(short i)
    {
        if (i < 0 || i > 3)
            throw std::out_of_range("index out of range");
        return arr_[i];
    }

    float vec4::norm()
    {
        return *this * *this;
    }

    vec4 &vec4::normalize()
    {
        *this *= (1. / norm());
        return *this;
    }

    vec4 &vec4::operator*=(float f)
    {
        for (short i = 0; i < 4; ++i)
            arr_[i] *= f;

        return *this;
    }

    float operator*(const vec4 &lhs, const vec4 &rhs)
    {
        float sum = 0;
        for (short i = 0; i < 4; ++i)
            sum += lhs.arr_[i] * rhs.arr_[i];
        return sum;
    }

    vec4 cross3(const vec4 &lhs, const vec4 &rhs)
    {
        return vec4({ lhs.at(1) * rhs.at(2) - lhs.at(2) * rhs.at(1),
                      lhs.at(2) * rhs.at(0) - lhs.at(0) * rhs.at(2),
                      lhs.at(0) * rhs.at(1) - lhs.at(1) * rhs.at(0), 0 });
    }
} // namespace mygl