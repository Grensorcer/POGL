#include "shadow_map.hh"
#include <iostream>

namespace mygl
{
    bool ShadowMap::init(unsigned int width, unsigned int height)
    {
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

    void ShadowMap::write()
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);
    }

    void ShadowMap::read(GLenum unit)
    {
        glActiveTexture(unit);
        glBindTexture(GL_TEXTURE_2D, buffer_);
    }
} // namespace mygl