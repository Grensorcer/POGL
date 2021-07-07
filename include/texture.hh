#pragma once
#include <GL/glew.h>

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