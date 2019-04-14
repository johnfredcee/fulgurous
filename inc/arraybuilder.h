
#pragma once

// GLAD
//#include <glad/glad.h>

#include <tuple>
#include <vector>
#include <string>
#include "bufferbuilder.h"
#include "drawcall.h"

template <typename T>
using BufferPtr = std::shared_ptr<BufferBuilder<T>>;

template<typename T>
using BufferInitialiser = std::tuple<std::string, BufferBuilder<T>, GLenum, GLenum>;

template<typename T>
using UniformInitialiser = std::tuple<std::string, T>;

template< typename T >
auto BuildVAOBuffer(std::shared_ptr<ShaderProgram> program, BufferInitialiser<T> t)
{
    using BT = Buffer<typename T>;
	auto&[name, builder, array_type, element_type] = t;
    std::shared_ptr<typename BT> buffer = builder.make_buffer(array_type, element_type);
    std::string parameterName(name);
    if (!parameterName.empty()) {
        buffer->bindAttribute(program, parameterName);
    } else {
        buffer->bindIndices();
    }
    return buffer;
}

template<typename T> 
auto BuildBuffers(T t)
{

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


template <typename T>
auto addBufferToDrawCall(std::shared_ptr<DrawCall> call,  BufferInitialiser<T> t)
{
    using BT = Buffer<typename T>;
	auto&[name, builder, array_type, element_type] = t;
    std::shared_ptr<typename BT> buffer = builder.make_buffer(array_type, element_type);
    std::string parameterName(name);
    if (!parameterName.empty()) {
        call->addBuffer(parameterName, buffer);
    } else {
    	call->>bindIndices(buffer);
    }
}

template <class... Ts>
void bufferAggregator(std::shared_ptr<DrawCall> call, Ts... Ts)
{
	auto t = std::make_tuple(addBufferToDrawCall(call, ts)...);
}

template <typename T>
auto addUniformToDrawCall(std::shared_ptr<DrawCall> call, T t)
{
   std::string parameterName(std::get<0>(t));
   call->addUniform(parameterName, std::get<1>(t));
}

template <class... Ts>
void uniformAggregator(std::shared_ptr<DrawCall> call, Ts... Ts)
{
	auto t = std::make_tuple(addUniformToDrawCall(call, ts)...);
}