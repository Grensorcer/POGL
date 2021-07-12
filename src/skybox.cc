#include "skybox.hh"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace mygl
{
    CubeMap::CubeMap(const std::string &dir, const std::string &left,
                     const std::string &right, const std::string &top,
                     const std::string &bottom, const std::string &front,
                     const std::string &back)
    {
        names_[0] = dir + left;
        names_[1] = dir + right;
        names_[2] = dir + top;
        names_[3] = dir + bottom;
        names_[4] = dir + front;
        names_[5] = dir + back;
    }

    bool CubeMap::load()
    {
        const GLenum GL_TEXTURE_TYPES[6] = {
            GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };

        glGenTextures(1, &buffer_);
        glBindTexture(GL_TEXTURE_CUBE_MAP, buffer_);

        for (int i = 0; i < 6; ++i)
        {
            int width, height, n;
            stbi_set_flip_vertically_on_load(true);
            unsigned char *data =
                stbi_load(names_[i].c_str(), &width, &height, &n, 0);
            if (!data)
            {
                std::cerr << "Could not load texture " << names_[i] << '\n';
                return false;
            }
            glTexImage2D(GL_TEXTURE_TYPES[i], 0, GL_RGB, width, height, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, data);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER,
                            GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                            GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,
                            GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,
                            GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,
                            GL_CLAMP_TO_EDGE);
            stbi_image_free(data);
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        return true;
    }

    void CubeMap::bind(GLenum unit)
    {
        glActiveTexture(unit);
        glBindFramebuffer(GL_TEXTURE_CUBE_MAP, buffer_);
    }
} // namespace mygl