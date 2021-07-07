#pragma once
#include <GL/glew.h>
#include <string>

namespace mygl
{
    class Texture
    {
    public:
        Texture(const char *name);
        bool load();
        void bind(GLenum unit);

    private:
        const char *name_;
        GLuint buffer_;
    };
} // namespace mygl