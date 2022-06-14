

#include <glad/glad.h>

#include <cstdio>
#include <memory>
#include <ios>
#include <fstream>
#include <filesystem>

using ios = std::ios;

std::streamoff file_length(std::ifstream &file)
{
    if (!file.good())
        return 0;

    std::streamoff pos = file.tellg();
    file.seekg(0, ios::end);
    std::streamoff len = file.tellg();
    file.seekg(ios::beg);

    return len;
}

std::shared_ptr<GLchar[]> load_shader(const std::filesystem::path& filename)
{
    std::shared_ptr<GLchar[]> result;
    std::ifstream file;
    file.open(filename, ios::in); // opens as ASCII!
    if (!file)
        return result;

    std::streamoff len = file_length(file);

    if (len == 0)
    {
        return result; // Error: Empty File
    }

    result = std::shared_ptr<GLchar[]>(new GLchar[len + 1]);

    char *ShaderSource = result.get();

    // len isn't always strlen cause some characters are stripped in ascii read...
    // it is important to 0-terminate the real length later, len is just max possible value...
    ShaderSource[len] = '\0';

    unsigned int i = 0;
    while (file.good())
    {
        ShaderSource[i] = file.get(); // get character from file.
        if (!file.eof())
            i++;
    }

    ShaderSource[i] = 0; // 0-terminate it at the correct position

    file.close();

    return result; // No Error
}
