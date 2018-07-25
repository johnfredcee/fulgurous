
#pragma once

/**
 * Templates to map OpenGL enums to their respective types
 */


template <GLenum T>
struct GL_type
{
};

template <typename T>
struct GL_enum
{
};

template <>
struct GL_type<GL_FLOAT> 
{
    using type = GLfloat;
};

template <>
struct GL_enum<GLfloat>
{
    static constexpr GLenum value = GL_FLOAT;
};

template <>
struct GL_type<GL_DOUBLE>
{
    using type = GLdouble;
};

template <>
struct GL_enum<GLdouble>
{
    static constexpr GLenum value = GL_DOUBLE;
};

template <>
struct GL_type <GL_UNSIGNED_INT>
{
    using type = GLuint;
};

template <>
struct GL_enum<GLuint>
{
    static constexpr GLenum value = GL_UNSIGNED_INT;
};

template <>
struct GL_type<GL_UNSIGNED_SHORT>
{
    using type = GLushort;
};

template <>
struct GL_enum<GLushort>
{
    static constexpr GLenum value = GL_UNSIGNED_SHORT;
};

template <>
struct GL_type<GL_UNSIGNED_BYTE>
{
    using type = GLubyte;
};

template <>
struct GL_enum<GLubyte>
{
    static constexpr GLenum value = GL_UNSIGNED_BYTE;
};

template <>
struct GL_type <GL_INT>
{
    using type = GLint;
};

template <>
struct GL_enum<GLint>
{
    static constexpr GLenum value = GL_INT;
};

template <>
struct GL_type<GL_SHORT>
{
    using type = GLshort;
};

template <>
struct GL_enum<GLshort>
{
    static constexpr GLenum value = GL_SHORT;
};

template <>
struct GL_type<GL_BYTE>
{
    using type = GLbyte;
};

template <>
struct GL_enum<GLbyte>
{
    static constexpr GLenum value = GL_BYTE;
};

template <GLenum T, GLsizei C>
struct GL_element_type
{
    using element = typename GL_type<T>::type;
    using type = element[C];
};
