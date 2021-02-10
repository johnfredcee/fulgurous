
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
using AttributeInitaliser = std::tuple<GLint, std::shared_ptr<Buffer<T>> >;

template< typename T >
AttributeInitaliser<T> build_data_buffer(std::shared_ptr<ShaderProgram> program, BufferInitialiser<T> t)
{
    using BT = Buffer<typename T>;
	auto&[name, builder, array_type, element_type] = t;
    std::shared_ptr<typename BT> buffer = builder.make_buffer(array_type, element_type);
    return std::make_tuple(program->attribute_location(name), buffer);
}

template< typename T >
void bind_attribute(AttributeInitaliser<T>& attribute_buffer)
{
	auto& [location, buffer] = attribute_buffer;
	if (location != -1)
	{
		buffer->bindAttribute(location);
	}
	else
	{
		buffer->bindIndices();
	}
}

template <typename... Ts>
void bind_attributes(std::tuple<Ts...>& tuple) {
    std::apply([](auto&... attribute_buffer)
				{	
					(bind_attribute(attribute_buffer), ...);
				},
				tuple);
}


template <class... Ts>
void array_builder(GLuint& vaoID, std::shared_ptr<ShaderProgram> program, Ts... ts)
{
	auto t = std::make_tuple(build_data_buffer(program, ts)...);
    gl_exec(glGenVertexArrays, 1, &vaoID);
    gl_exec(glBindVertexArray, vaoID);
	bind_attributes(t);
    gl_exec(glBindVertexArray, 0);
    return;
}

#if 0

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

#endif