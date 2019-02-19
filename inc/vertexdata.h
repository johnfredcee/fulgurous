#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "buffer.h"

#include <tuple>      // std::tuple
#include <functional> // std::invoke

class VertexAttribute
{
	friend class VertexData;
public:
	GLenum  type;
	GLsizei offset;
	GLsizei count;
	void    *data;
};

template<typename T>
struct VertexAttributeInitialiser
{
	using element_type = typename T;
	using component_type = typename T::type;

	const std::string name;
	GLsizei count;
	static constexpr GLsizei componentCount = element_type::dim;
	static constexpr GLenum  type = GL_enum<component_type>::value;
};

class VertexData
{
 	void    *data;
	 GLsizei size;
	std::unordered_map<std::string, VertexAttribute> attributes;
};

 template<typename T>
 GLsizei addAttributeToVertexData(const VertexAttributeInitialiser<typename T>& initializer, GLsizei offset)
 {
	VertexAttribute attribute{ initializer.type, offset, initializer.count, nullptr };
	attributes[initializer.name] = attribute;
	constexpr GLsizei typeSize = sizeof(initializer.type);
	return offset +  count * typeSize * initializer.count;
 }

 
 