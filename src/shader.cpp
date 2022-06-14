

#include <glad/glad.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <filesystem>
#include <utils.h>
#include <gl_funcalls.h>
#include "shader.h"

Shader::Shader(ShaderKind inKind) : kind(inKind)
{
}

Shader::~Shader()
{
}

void Shader::load_from_file(const std::filesystem::path &filename)
{
    std::shared_ptr<GLchar[]> source = load_shader(filename);
    load_from_string(source.get());
}

void Shader::load_from_string(const std::string &source)
{
    const GLuint glShaderKind = glShaderConstants[kind];
    // shaders[kind] = glCreateShader(glShaderConstants[kind]);
    GLint source_length = (GLint)source.size();
    GLchar *source_text = (GLchar *)source.c_str();
    gl_exec(glShaderSource, glShaderKind, 1, &source_text, &source_length);
    return;
}

void Shader::compile()
{
    const GLuint glShaderKind = glShaderConstants[kind];
    gl_exec(glCompileShader, glShaderKind);
    GLint result;
    gl_exec(glGetShaderiv, glShaderKind, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        std::cerr << "Shader compilation failed." << std::endl;
        GLint loglen;
        gl_exec(glGetShaderiv, glShaderKind, GL_INFO_LOG_LENGTH, &loglen);
        if (loglen > 0)
        {
            std::shared_ptr<GLchar[]> log(new GLchar[loglen]);
            GLsizei written;
            gl_exec(glGetShaderInfoLog, glShaderKind, loglen, &written, log.get());
            std::cerr << "Shader log." << std::endl;
            std::cerr << log << std::endl;
            DebugBreak();
            return;
        }
    }
}

bool Shader::verify()
{
    return true;
}

