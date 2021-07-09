#include "texture.hh"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

namespace mygl
{
    Texture::Texture(const char *name)
        : name_{ name }
    {}

    bool Texture::load()
    {
        glGenTextures(1, &buffer_);
        glBindTexture(GL_TEXTURE_2D, buffer_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, n;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load(name_, &width, &height, &n, 0);
        if (!data)
        {
            std::cerr << "Could not load texture " << name_ << '\n';
            return false;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
        return true;
    }

    void Texture::bind(GLenum unit)
    {
        glActiveTexture(unit);
        glBindTexture(GL_TEXTURE_2D, buffer_);
    }

} // namespace mygl