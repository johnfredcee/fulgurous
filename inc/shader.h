
#pragma once

enum ShaderKind : GLuint
{
    eVERTEX_SHADER  =  0,
    eFRAGMENT_SHADER = 1,
    eGEOMETRY_SHADER = 2,
    eSHADER_COUNT    = 3  
};

class Shader
{
public:
    Shader(ShaderKind inKind);
    ~Shader();
    void load_from_file(const std::filesystem::path& filename);
    void load_from_string(const std::string& filename);
    void compile();
    bool verify();
protected:
    ShaderKind kind;
    const GLuint glShaderConstants[ShaderKind::eSHADER_COUNT] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER };

};