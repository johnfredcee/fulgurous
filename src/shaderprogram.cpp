
#include <glad/glad.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include <filesystem/path.h>
#include <utils.h>
#include <gl_funcalls.h>
#include <shader.h>
#include <shaderprogram.h>

ShaderProgram::ShaderProgram()
: program(0)
{
    shaders[ShaderKind::eVERTEX_SHADER]   = 0;
    shaders[ShaderKind::eFRAGMENT_SHADER] = 0;
    shaders[ShaderKind::eGEOMETRY_SHADER] = 0;
}

ShaderProgram::~ShaderProgram()
{
    if (program != 0)
    {
        gl_exec(glDeleteProgram, program);
        program = 0;
    }
}

void ShaderProgram::use()
{
    if (program != 0)
    {
        gl_exec(glUseProgram, program);
    }
}

void ShaderProgram::unuse()
{
    gl_exec(glUseProgram, 0);
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
    GLint source_length = (GLint) source.size();
    GLchar *source_text =  (GLchar*) source.c_str();
    gl_exec(glShaderSource, shaders[kind], 1, &source_text, &source_length);
    return;
}

void ShaderProgram::compile(ShaderKind kind)
{
    if (shaders[kind] != 0)
    {
        gl_exec(glCompileShader, shaders[kind]);
        GLint result;
        gl_exec(glGetShaderiv, shaders[kind], GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            std::cerr << "Shader compilation failed." << std::endl;
            GLint loglen;
            gl_exec(glGetShaderiv, shaders[kind], GL_INFO_LOG_LENGTH, &loglen );
            if (loglen > 0)
            {
                std::shared_ptr<GLchar[]> log(new GLchar[loglen]);
                GLsizei written;
                gl_exec(glGetShaderInfoLog, shaders[kind], loglen, &written, log.get());
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
    gl_exec(glGetProgramiv, program, GL_ACTIVE_ATTRIBUTES, &num_attributes);
    GLint max_attribute_name_length;
    gl_exec(glGetProgramiv, program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_attribute_name_length);
    for(GLint attribute_index = 0; attribute_index < num_attributes; ++attribute_index)
    {
        ShaderParameter attribute;
        GLint real_attribute_name_length;
        GLint attribute_byte_size;
        GLenum attribute_type;
        GLchar *attribute_name = new GLchar[max_attribute_name_length+1];
        memset(attribute_name, 0, max_attribute_name_length);
        gl_exec(glGetActiveAttrib, program, attribute_index, max_attribute_name_length, &real_attribute_name_length, &attribute_byte_size, &attribute_type, attribute_name);
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
    gl_exec(glGetProgramiv, program, GL_ACTIVE_UNIFORMS, &num_uniforms);
    GLint max_uniform_name_length;
    gl_exec(glGetProgramiv, program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uniform_name_length);
    for(GLint uniform_index = 0; uniform_index < num_uniforms; ++uniform_index)
    {
        ShaderParameter uniform;
        GLint real_uniform_name_length;
        GLint uniform_byte_size;
        GLenum uniform_type;
        GLchar *uniform_name = new GLchar[max_uniform_name_length+1];
        memset(uniform_name, 0, max_uniform_name_length);
        gl_exec(glGetActiveUniform, program, uniform_index, max_uniform_name_length, &real_uniform_name_length, &uniform_byte_size, &uniform_type, uniform_name);
        GLint uniform_location = glGetUniformLocation(program, uniform_name);
        uniform.location = uniform_location;
        uniform.name = std::string(uniform_name);
        uniforms.push_back(uniform);
    }       
}

GLint ShaderProgram::attribute_location(const std::string& name)
{
    auto result = std::find_if(std::begin(attributes), std::end(attributes), [name](const ShaderParameter& param) { return param.name == name; });
    return result != std::end(attributes) ? result->location : -1;
}

GLint ShaderProgram::uniform_location(const std::string& name)
{
    auto result = std::find_if(std::begin(uniforms), std::end(uniforms), [name](const ShaderParameter& param) { return param.name == name; });
    return result != std::end(uniforms) ? result->location : -1;
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
                gl_exec(glAttachShader, program, shaders[i]);
            }
        }
        gl_exec(glLinkProgram, program);
        GLint result;
        gl_exec(glGetProgramiv, program, GL_LINK_STATUS, &result);
        if (result == GL_FALSE)
        {
            std::cerr << "Program linking failed." << std::endl;
            GLint loglen;
            gl_exec(glGetProgramiv, program, GL_INFO_LOG_LENGTH, &loglen );
            if (loglen > 0)
            {
                std::shared_ptr<GLchar[]> log(new GLchar[loglen]);
                GLsizei written;
                gl_exec(glGetProgramInfoLog, program, loglen, &written, log.get());
                std::cerr << "Program log." << std::endl;
                std::cerr << log << std::endl;            
                DebugBreak();
                return;
            }           
        }
        for(unsigned i = 0; i < eSHADER_COUNT; i++)
        {
            if (shaders[i] != 0)
            {
                gl_exec(glDetachShader, program, shaders[i]);
            }
        }
        for(unsigned i = 0; i < eSHADER_COUNT; i++)
        {
            if (shaders[i] != 0)
            {
                gl_exec(glDeleteShader, shaders[i]);
            }
        }
        gl_exec(glUseProgram, program);
        gather_attributes();
        gather_uniforms();
    }
}
