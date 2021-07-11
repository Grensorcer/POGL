#pragma once
#include <GL/glew.h>

namespace mygl
{
    class ShadowMap
    {
    public:
        bool init(unsigned int width, unsigned int height);
        void write();
        void read(GLenum unit);

    private:
        GLuint fbo_;
        GLuint buffer_;
    };
} // namespace mygl