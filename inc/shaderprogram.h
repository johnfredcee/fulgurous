
#pragma once

class ShaderProgram
{
public:
    struct ShaderParameter
    {
        GLuint location;
        std::string name;
    };
    ShaderProgram(); 
    ~ShaderProgram();

    void use();    
    void unuse();

    void load_from_string(ShaderKind kind,  const std::string& source);
    void load_from_file(ShaderKind kind, const std::string& filename);

    void compile(ShaderKind kind);

    void link();

    GLint attribute_location(const std::string& name);
    GLint uniform_location(const std::string& name);
    
    std::vector<ShaderParameter> uniforms;
    std::vector<ShaderParameter> attributes;
    
private:

    void gather_attributes();
    void gather_uniforms();

    GLuint shaders[eSHADER_COUNT];
    GLuint program;
};