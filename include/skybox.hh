#pragma once
#include <string>
#include <GL/glew.h>
#include "program.hh"

namespace mygl
{
    class CubeMap
    {
    public:
        CubeMap(const std::string &dir, const std::string &left,
                const std::string &right, const std::string &top,
                const std::string &bottom, const std::string &front,
                const std::string &back);
        bool load();
        void bind(GLenum unit);

    private:
        std::string names_[6];
        GLuint buffer_;
    };

    class SkyBox
    {
    public:
    private:
        CubeMap texture_;
        program program_;
    };
} // namespace mygl