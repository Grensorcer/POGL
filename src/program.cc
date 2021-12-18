#include "program.hh"

namespace mygl
{
    program::program()
        : id_{ glCreateProgram() }
    {}

    program::~program()
    {
        glDeleteProgram(id_);
    }

    bool program::add_shader(const std::string &src, GLuint shader_type)
    {
        GLint is_ok, logs_length;
        char *tmp_logs;

        const auto csrc = src.c_str();
        auto shader_id = glCreateShader(shader_type);
        shader_ids.push_back(shader_id);
        glShaderSource(shader_id, 1, &csrc, NULL);
        glCompileShader(shader_id);

        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &logs_length);
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &is_ok);
        tmp_logs = new char[logs_length];
        glGetShaderInfoLog(shader_id, logs_length, NULL, tmp_logs);
        append_log(tmp_logs);
        delete[] tmp_logs;

        return is_ok;
    }

    bool program::link()
    {
        GLint is_ok, logs_length;
        char *tmp_logs;

        for (const auto shader_id : shader_ids)
            glAttachShader(id_, shader_id);

        glLinkProgram(id_);

        for (const auto shader_id : shader_ids)
            glDeleteShader(shader_id);
        shader_ids.clear();

        // Check linking problems
        glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &logs_length);
        glGetProgramiv(id_, GL_LINK_STATUS, &is_ok);
        tmp_logs = new char[logs_length];
        glGetProgramInfoLog(id_, logs_length, NULL, tmp_logs);
        append_log(tmp_logs);
        delete[] tmp_logs;

        return is_ok;
    }

    std::shared_ptr<program>
    program::make_compute(std::string &compute_shader_src)
    {
        // Initialize program and vars
        auto p = std::make_shared<program>();
        if (!p->add_shader(compute_shader_src, GL_COMPUTE_SHADER))
            return p;

        p->ready_ = p->link();
        return p;
    }

    std::shared_ptr<program>
    program::make_program(const std::map<GLuint, std::string> &shader_sources)
    {
        // Initialize program and vars
        auto p = std::make_shared<program>();
        for (const auto &shader : shader_sources)
            if (!p->add_shader(shader.second, shader.first))
                return p;

        p->ready_ = p->link();
        return p;
    }

    const char *program::get_log() const
    {
        return logs_.c_str();
    }

    const GLuint &program::id() const
    {
        return id_;
    }

    bool program::is_ready() const
    {
        return ready_;
    }

    void program::use() const
    {
        glUseProgram(id_);
    }

    void program::append_log(char *logs)
    {
        logs_ += logs;
    }

    void program::set_int(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(id_, name.c_str()), value);
    }

    void program::set_uint(const std::string &name, unsigned int value) const
    {
        glUniform1ui(glGetUniformLocation(id_, name.c_str()), value);
    }

    void program::set_float(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(id_, name.c_str()), value);
    }

    void program::set_vec3(const std::string &name,
                           const glm::vec3 &value) const
    {
        glUniform3fv(glGetUniformLocation(id_, name.c_str()), 1, &value[0]);
    }

    void program::set_mat4(const std::string &name,
                           const glm::mat4 &value) const
    {
        glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, false,
                           &value[0][0]);
    }
} // namespace mygl