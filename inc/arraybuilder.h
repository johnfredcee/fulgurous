
#pragma once

// GLAD
//#include <glad/glad.h>

#include <tuple>
#include <vector>
#include "bufferbuilder.h"

template <typename T>
using BufferPtr = std::shared_ptr<BufferBuilder<T>>;

template<typename T>
using BufferInitialiser = std::tuple<BufferBuilder<T>, GLenum, GLenum>;


template< typename T >
auto BuildVAOBuffer(T t)
{
    using B = std::tuple_element<0,T>::type;
    using BT = Buffer<typename B::element_type>;
    std::shared_ptr<typename BT> buffer = std::get<0>(t).make_buffer(std::get<1>(t), std::get<2>(t));
    return buffer;
}

template <class... Ts>
auto arrayBuilder(GLuint& vaoID, Ts... ts)
{
    gl_exec(glGenVertexArrays, 1, &vaoID);
    gl_exec(glBindVertexArray, vaoID);

    std::initializer_list<int> dummy{(BuildVAOBuffer(ts),0)...};
    auto t = std::make_tuple(BuildVAOBuffer(ts)...);
    gl_exec(glBindVertexArray, 0);
    return t;
}
