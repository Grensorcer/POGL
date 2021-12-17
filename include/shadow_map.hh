#pragma once
#include <GL/glew.h>

namespace mygl
{
    class ShadowMap
    {
    public:
        virtual bool init(unsigned int width, unsigned int height) = 0;
        void write();
        virtual void read(GLenum unit) = 0;

    protected:
        GLuint fbo_;
        GLuint buffer_;
    };

    class DirectionalShadowMap : public ShadowMap
    {
    public:
        bool init(unsigned int width, unsigned int height) override;
        void read(GLenum unit) override;
    };

    class CubeShadowMap : public ShadowMap
    {
    public:
        bool init(unsigned int width, unsigned int height) override;
        void read(GLenum unit) override;
    };
} // namespace mygl