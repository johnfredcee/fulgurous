
#include <glad/glad.h>
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <filesystem/path.h>
#include <utils.h>
#include <shader.h>

ShaderProgram::ShaderProgram()
: program(0)
{
    shaders[ShaderKind::eFRAGMENT_SHADER] = 0;
    shaders[ShaderKind::eVERTEX_SHADER]   = 0;
    shaders[ShaderKind::eFRAGMENT_SHADER] = 0;
}

ShaderProgram::~ShaderProgram()
{
    if (program != 0)
    {
        glDeleteProgram(program);
        program = 0;
    }
}

void ShaderProgram::use()
{
    if (program != 0)
    {
        glUseProgram(program);
    }
}

void ShaderProgram::unuse()
{
    glUseProgram(0);
}

void ShaderProgram::load_from_file(ShaderKind kind, const std::string& filename)
{
    std::shared_ptr<GLchar[]> source = load_shader(filename.c_str());
    load_from_string(kind, source.get());    
}

void ShaderProgram::load_from_string(ShaderKind kind, const std::string& source)
{
    GLuint glShaderConstants[ShaderKind::eSHADER_COUNT] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER };
    shaders[kind] = glCreateShader(glShaderConstants[kind]);
    GLint source_length = source.size();
    GLchar *source_text =  (GLchar*) source.c_str();
    glShaderSource(shaders[kind], 1, &source_text, &source_length);
    return;
}

void ShaderProgram::compile(ShaderKind kind)
{
    if (shaders[kind] != 0)
    {
        glCompileShader(shaders[kind]);
        GLint result;
        glGetShaderiv(shaders[kind], GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            std::cerr << "Shader compilation failed." << std::endl;
            GLint loglen;
            glGetShaderiv(shaders[kind], GL_INFO_LOG_LENGTH, &loglen );
            if (loglen > 0)
            {
                std::shared_ptr<GLchar[]> log(new GLchar[loglen]);
                GLsizei written;
                glGetShaderInfoLog(shaders[kind], loglen, &written, log.get());
                std::cerr << "Shader log." << std::endl;
                std::cerr << log << std::endl;            
                DebugBreak();
                return;
            }
        }
    }
}

void ShaderProgram::gather_attributes()
{
    GLint num_attributes;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &num_attributes);
    GLint max_attribute_name_length;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_attribute_name_length);
    for(GLint attribute_index = 0; attribute_index < num_attributes; ++attribute_index)
    {
        ShaderParameter attribute;
        GLint real_attribute_name_length;
        GLint attribute_byte_size;
        GLenum attribute_type;
        GLchar *attribute_name = new GLchar[max_attribute_name_length+1];
        memset(attribute_name, 0, max_attribute_name_length);
        glGetActiveAttrib(program, attribute_index, max_attribute_name_length, &real_attribute_name_length, &attribute_byte_size, &attribute_type, attribute_name);
        GLint attribute_location = glGetAttribLocation(program, attribute_name);
        attribute.location = attribute_location;
        attribute.name = std::string(attribute_name);
        attributes.push_back(attribute);
        delete [] attribute_name;
    }
}

void ShaderProgram::gather_uniforms()
{
    GLint num_uniforms;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &num_uniforms);
    GLint max_uniform_name_length;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uniform_name_length);
    for(GLint uniform_index = 0; uniform_index < num_uniforms; ++uniform_index)
    {
        ShaderParameter uniform;
        GLint real_uniform_name_length;
        GLint uniform_byte_size;
        GLenum uniform_type;
        GLchar *uniform_name = new GLchar[max_uniform_name_length+1];
        memset(uniform_name, 0, max_uniform_name_length);
        glGetActiveUniform(program, uniform_index, max_uniform_name_length, &real_uniform_name_length, &uniform_byte_size, &uniform_type, uniform_name);
        GLint uniform_location = glGetUniformLocation(program, uniform_name);
        uniform.location = uniform_location;
        uniform.name = std::string(uniform_name);
        uniforms.push_back(uniform);
    }       
}

void ShaderProgram::link()
{
    if ((shaders[eVERTEX_SHADER] != 0) && (shaders[eFRAGMENT_SHADER] != 0))
    {
        program = glCreateProgram();
        if (program == 0)        
        {
            std::cerr << "Program creation failed." << std::endl;
            return;
        }
        for(unsigned i = 0; i < eSHADER_COUNT; i++)
        {
            if (shaders[i] != 0)
            {
                glAttachShader(program, shaders[i]);
            }
        }
        glLinkProgram(program);
        GLint result;
        glGetProgramiv(program, GL_LINK_STATUS, &result);
        if (result == GL_FALSE)
        {
            std::cerr << "Program linking failed." << std::endl;
            GLint loglen;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &loglen );
            if (loglen > 0)
            {
                std::shared_ptr<GLchar[]> log(new GLchar[loglen]);
                GLsizei written;
                glGetProgramInfoLog(program, loglen, &written, log.get());
                std::cerr << "Program log." << std::endl;
                std::cerr << log << std::endl;            
                DebugBreak();
                return;
            }           
        }
        glUseProgram(program);
        gather_attributes();
        gather_uniforms();
    }
}
