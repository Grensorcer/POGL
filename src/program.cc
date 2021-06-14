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

    program program::make_program(std::string &vs_src, std::string &fs_src)
    {
        // Initialize program and vars
        program p;
        GLint is_ok, logs_length;
        char *tmp_logs;

        // Create and compile vertex shader
        const auto vs_csrc = vs_src.c_str();
        GLuint vs_id_ = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs_id_, 1, &vs_csrc, NULL);
        glCompileShader(vs_id_);

        // Vertex shader error check
        glGetShaderiv(vs_id_, GL_INFO_LOG_LENGTH, &logs_length);
        glGetShaderiv(vs_id_, GL_COMPILE_STATUS, &is_ok);
        tmp_logs = new char[logs_length];
        glGetShaderInfoLog(vs_id_, logs_length, NULL, tmp_logs);
        p.append_log(tmp_logs);
        delete[] tmp_logs;

        if (!is_ok)
            return p;

        // Create and compile fragment shader
        const auto fs_csrc = fs_src.c_str();
        GLuint fs_id_ = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs_id_, 1, &fs_csrc, NULL);
        glCompileShader(fs_id_);

        // Fragment shader error check
        glGetShaderiv(fs_id_, GL_INFO_LOG_LENGTH, &logs_length);
        glGetShaderiv(fs_id_, GL_COMPILE_STATUS, &is_ok);
        tmp_logs = new char[logs_length];
        glGetShaderInfoLog(fs_id_, logs_length, NULL, tmp_logs);
        p.append_log(tmp_logs);
        delete[] tmp_logs;
        if (!is_ok)
            return p;

        // Link everything
        glAttachShader(p.id_, vs_id_);
        glAttachShader(p.id_, fs_id_);
        glLinkProgram(p.id_);

        glDeleteShader(vs_id_);
        glDeleteShader(fs_id_);

        // Check linking problems
        glGetProgramiv(p.id_, GL_INFO_LOG_LENGTH, &logs_length);
        glGetProgramiv(p.id_, GL_LINK_STATUS, &is_ok);
        tmp_logs = new char[logs_length];
        glGetProgramInfoLog(p.id_, logs_length, NULL, tmp_logs);
        p.append_log(tmp_logs);
        delete[] tmp_logs;
        if (!is_ok)
            return p;

        p.ready_ = true;
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
} // namespace mygl