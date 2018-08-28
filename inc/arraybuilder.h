
#pragma once

// GLAD
//#include <glad/glad.h>

#include <tuple>
#include <vector>
#include "bufferbuilder.h"

template <typename T>
using BufferPtr = std::shared_ptr<BufferBuilder<T>>;

template<typename T>
using BufferInitialiser = std::tuple<std::string, BufferBuilder<T>, GLenum, GLenum>;


template< typename T >
auto BuildVAOBuffer(std::shared_ptr<ShaderProgram> program, T t)
{
    using B = std::tuple_element<1,T>::type;
    using BT = Buffer<typename B::element_type>;
    std::shared_ptr<typename BT> buffer = std::get<1>(t).make_buffer(std::get<2>(t), std::get<3>(t));
    std::string parameterName(std::get<0>(t));
    if (!parameterName.empty()) {
        buffer->bindAttribute(program, parameterName);
    } else {
        buffer->bindIndices();
    }

    return buffer;
}

template <class... Ts>
void arrayBuilder(GLuint& vaoID, std::shared_ptr<ShaderProgram> program, Ts... ts)
{
    gl_exec(glGenVertexArrays, 1, &vaoID);
    gl_exec(glBindVertexArray, vaoID);

    auto t = std::make_tuple(BuildVAOBuffer(program, ts)...);
    gl_exec(glBindVertexArray, 0);
    return;
}
