#include "shadow_map.hh"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <iostream>

namespace mygl
{

    void ShadowMap::write()
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);
    }

    bool DirectionalShadowMap::init(unsigned int width, unsigned int height)
    {
        projection =
            glm::perspective(glm::radians(45.f), 1920.f / 1080.f, 0.1f, 1000.f);

        glGenFramebuffers(1, &fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

        glGenTextures(1, &buffer_);
        glBindTexture(GL_TEXTURE_2D, buffer_);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, buffer_, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        bool res = err == GL_FRAMEBUFFER_COMPLETE;
        if (!res)
            std::cerr << "FrameBuffer status incomplete: " << err << '\n';

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return res;
    }

    void DirectionalShadowMap::read(GLenum unit)
    {
        glActiveTexture(unit);
        glBindTexture(GL_TEXTURE_2D, buffer_);
    }

    bool CubeShadowMap::init(unsigned int width, unsigned int height)
    {
        glGenFramebuffers(1, &fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

        glGenTextures(1, &buffer_);
        glBindTexture(GL_TEXTURE_CUBE_MAP, buffer_);
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,
                        GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,
                        GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,
                        GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        for (short i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                         GL_DEPTH_COMPONENT, width, height, 0,
                         GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }

        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, buffer_, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        bool res = err == GL_FRAMEBUFFER_COMPLETE;
        if (!res)
            std::cerr << "FrameBuffer status incomplete: " << err << '\n';

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return res;
    }

    void CubeShadowMap::read(GLenum unit)
    {
        glActiveTexture(unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, buffer_);
    }
} // namespace mygl