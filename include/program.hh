#pragma once
#include <string>
#include <memory>
#include <GL/glew.h>
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

namespace mygl
{
    class program
    {
    public:
        program();
        program(GLuint id, GLuint vs_id, GLuint fs_id);
        ~program();
        static std::shared_ptr<program>
        make_program(std::string &vertex_shader_src,
                     std::string &fragment_shader_src);
        static std::shared_ptr<program> make_compute(std::string &cs_src);

        const char *get_log() const;
        const GLuint &id() const;
        bool is_ready() const;
        void use() const;

        void set_int(const std::string &name, int value) const;
        void set_float(const std::string &name, float value) const;
        void set_vec3(const std::string &name, glm::vec3 value) const;
        void set_mat4(const std::string &name, glm::mat4 value) const;

    private:
        bool ready_ = false;
        std::string logs_;
        GLuint id_;

        void append_log(char *logs);
    };
} // namespace mygl