#pragma once
#include <GL/glew.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

namespace mygl
{
    class ShadowMap
    {
    public:
        virtual bool init(unsigned int width, unsigned int height) = 0;
        void write();
        virtual void read(GLenum unit) = 0;

        const glm::mat4 &get_projection()
        {
            return projection;
        }

    protected:
        GLuint fbo_;
        GLuint buffer_;
        glm::mat4 projection;
    };

    class DirectionalShadowMap : public ShadowMap
    {
    public:
        bool init(unsigned int width, unsigned int height) override;
        void read(GLenum unit) override;

        const glm::mat4 &get_view()
        {
            return view;
        }

        void set_view(const glm::vec3 &view_position)
        {
            this->view = glm::lookAt(view_position, glm::vec3(0, 0, 0),
                                     glm::vec3(0, 1, 0));
        }

    protected:
        glm::mat4 view;
    };

    class CubeShadowMap : public ShadowMap
    {
    public:
        bool init(unsigned int width, unsigned int height) override;
        void read(GLenum unit) override;

        const glm::mat4 &get_view(short i)
        {
            return views[i];
        }

        void set_view(const glm::vec3 &view_position)
        {
            views[0] = glm::lookAt(view_position,
                                   view_position + glm::vec3(1.0, 0.0, 0.0),
                                   glm::vec3(0.0, -1.0, 0.0));
            views[1] = glm::lookAt(view_position,
                                   view_position + glm::vec3(-1.0, 0.0, 0.0),
                                   glm::vec3(0.0, -1.0, 0.0));
            views[2] = glm::lookAt(view_position,
                                   view_position + glm::vec3(0.0, 1.0, 0.0),
                                   glm::vec3(0.0, 0.0, 1.0));
            views[3] = glm::lookAt(view_position,
                                   view_position + glm::vec3(0.0, -1.0, 0.0),
                                   glm::vec3(0.0, 0.0, -1.0));
            views[4] = glm::lookAt(view_position,
                                   view_position + glm::vec3(0.0, 0.0, 1.0),
                                   glm::vec3(0.0, -1.0, 0.0));
            views[5] = glm::lookAt(view_position,
                                   view_position + glm::vec3(0.0, 0.0, -1.0),
                                   glm::vec3(0.0, -1.0, 0.0));
        }

    protected:
        glm::mat4 views[6];
    };
} // namespace mygl