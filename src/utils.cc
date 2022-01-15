#include "utils.hh"
#include <fstream>
#include <exception>

std::ostream &operator<<(std::ostream &out, const glm::vec3 &v)
{
    return out << '(' << v.x << ", " << v.y << ", " << v.z << ")\n";
}

namespace utils
{
    std::string read_file_content(const char *filename)
    {
        auto fin = std::ifstream(filename, std::ios::in | std::ios::binary);
        if (!fin)
            throw(std::invalid_argument("Wrong file"));
        std::string contents;
        fin.seekg(0, std::ios::end);
        contents.resize(fin.tellg());
        fin.seekg(0, std::ios::beg);
        fin.read(&contents[0], contents.size());
        fin.close();
        return contents;
    }

    std::string gl_error_string(GLenum err)
    {
        switch (err)
        {
        // opengl 2 errors (8)
        case GL_NO_ERROR:
            return "GL_NO_ERROR";

        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";

        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";

        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";

        case GL_STACK_OVERFLOW:
            return "GL_STACK_OVERFLOW";

        case GL_STACK_UNDERFLOW:
            return "GL_STACK_UNDERFLOW";

        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";

        case GL_TABLE_TOO_LARGE:
            return "GL_TABLE_TOO_LARGE";

        // opengl 3 errors (1)
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";

        // gles 2, 3 and gl 4 error are handled by the switch above
        default:
            return "unknown error " + std::to_string(err);
        }
    }

    void GLAPIENTRY messageCallback(GLenum, GLenum type, GLuint,
                                    GLenum severity, GLsizei,
                                    const GLchar *message, const void *)
    {
        std::string msg = "Opengl: type: " + gl_error_string(type) + ' '
            + std::string((const char *)message) + '\n';

        switch (severity)
        {
        case (GL_DEBUG_SEVERITY_HIGH):
            std::cerr << msg;
            break;
        case (GL_DEBUG_SEVERITY_MEDIUM):
            std::cerr << msg;
            break;
        case (GL_DEBUG_SEVERITY_LOW):
            std::cerr << msg;
            break;
        case (GL_DEBUG_SEVERITY_NOTIFICATION):
            std::cerr << msg;
            break;
        default:
            std::cerr << msg;
        }
    }

} // namespace utils