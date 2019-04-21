#pragma once

template <typename T>
class Buffer {
public:
	using element_type = typename T;
	using component_type = typename T::type;

private:
	GLuint	mBuffer;
	GLuint	mTarget;
	GLsizei mSize;
	static constexpr GLsizei mComponentCount = element_type::dim;
	static constexpr GLenum  mType = GL_enum<component_type>::value;

public:

	/**
	 * Construct a vertex or index buffer
	 * @param target GL_ARRAY_BUFFER for a buffer of elements, GL_ELEMENT_ARRAY_BUFFER for a buffer of indices
	 * @param bufferData Pointer to the actual buffer data
	 * @param type Data type of the data eg. GL_FLOAT
	 * @param count Number of items in the buffer
	 * @param componentCount Number of components in the item (eg 3 for verts, 2 for uv)
	 * @param usage Usage hint
	 */
	Buffer(GLenum		target,
			const void  *bufferData,
			GLsizei		count,
			GLenum		usage)	: mTarget(target), mSize(count)
	{
		constexpr GLsizei typeSize = sizeof(component_type);
		gl_exec(glGenBuffers, 1, &mBuffer);
		gl_exec(glBindBuffer, mTarget, mBuffer);
		gl_exec(glBufferData, mTarget, count * typeSize * mComponentCount, bufferData, usage);
		gl_exec(glBindBuffer, mTarget, 0);
		return;
	}


	~Buffer() 
	{
		gl_exec(glDeleteBuffers, 1, &mBuffer);
	}
	
	GLuint getSize() const {
		return mSize;
	}

	GLenum getType() const {
		return mType;
	}

	void update(const void *bufferData, GLenum usage)
	{
		constexpr GLsizei typeSize = sizeof(component_type);
		gl_exec(glBindBuffer, mTarget, mBuffer);
		gl_exec(glBufferData, mTarget, mSize * typeSize * mComponentCount, bufferData, usage);
		gl_exec(glBindBuffer, mTarget, 0);
		return;
	}

 	 void bindAttribute(std::shared_ptr<ShaderProgram> program, const std::string& name) 
	 {
	    GLint location = program->attribute_location(name);
		gl_exec(glBindBuffer, mTarget, mBuffer);
		gl_exec(glEnableVertexAttribArray, location);
		gl_exec(glVertexAttribPointer, location, mComponentCount, mType, GL_FALSE, GLsizei(sizeof(component_type) * mComponentCount), nullptr);
	 }

	void bindAttribute(GLint location)
	 {
		gl_exec(glBindBuffer, mTarget, mBuffer);
		gl_exec(glEnableVertexAttribArray, location);
		gl_exec(glVertexAttribPointer, location, mComponentCount, mType, GL_FALSE, GLsizei(sizeof(component_type) * mComponentCount), nullptr);
	 }

	void unbind()
	{
		gl_ext(glBindBuffer, mTarget, 0);
	}

	void bindIndices() 
	{
		assert(mTarget == GL_ELEMENT_ARRAY_BUFFER);
		gl_exec(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, mBuffer);
	}

	/**
	 * Actually draw the contents of the buffer
	 * @param mode Primitive type to draw eg GL_TRIANGLES
	 */
	void draw(GLenum mode) const  
	{
		assert(mTarget == GL_ELEMENT_ARRAY_BUFFER);
		gl_exec(glDrawElements, mode, mSize, mType, (void*) 0);
	}

	/**
	 * Draw some of the buffer
	 * @param mode Primitive to use
	 * @param count Number of elements tor draw
	 */
	void drawSome(GLenum mode, GLuint count) const
	{
		assert(mTarget == GL_ELEMENT_ARRAY_BUFFER);
		gl_exec(glDrawElements, mode, count, mType, (void*) 0);
	}

	void drawRange(GLenum mode, GLuint count, GLuint offset) const
	{
		assert(mTarget == GL_ELEMENT_ARRAY_BUFFER);
		gl_exec(glDrawElements, mode, count, mType, (void*) (offset) ); 
	}

	void drawImmediate(GLenum mode) const  
	{
		assert(mTarget == GL_ARRAY_BUFFER);
		gl_exec(glDrawArrays, mode, 0, mSize);
	}

	void unbindIndices() 
	{
		assert(mTarget == GL_ELEMENT_ARRAY_BUFFER);
		gl_exec(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, 0);
	}

};

template<typename T>
std::shared_ptr< Buffer<typename T> > produce_buffer(GLenum		target,
                                 		          const void  *bufferData,
                                        		  GLsizei		count,
                                           		  GLenum		usage)
{
	return std::make_shared<Buffer<T>>(target, bufferData, count, usage);
}




// /**
//  * Bind the buffer to a shader attribute
//  * @param program shader program with attribute we wish to bind
//  * @param attributeIndex index of attribute associated with program
//  * @param componentCount number of components in data (eg 3 for vert, 2 for uv)
//  */
// template< T, C >
// void Buffer::bindAttribute(Program* program, GLint attributeLocation, bool normalized) 
// {
// 	(void) program;
// 	SDL_assert(mTarget == GL_ARRAY_BUFFER);
// 	GLsizei typeSize = getTypeSize(mType);
// 	glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
// 	glEnableVertexAttribArray(attributeLocation);		
// 	SDL_assert(glGetError() == GL_NO_ERROR);				
// 	glVertexAttribPointer(
// 		attributeLocation,						  	    /* attribute */
// 		mComponentCount,								/* component size */
// 		mType,											/* type */
// 		normalized ? GL_TRUE : GL_FALSE,				/* normalized? */
// 		typeSize * mComponentCount,						/* stride */
// 		(void*)0 );										/* array buffer offset */
// 	SDL_assert(glGetError() == GL_NO_ERROR);				
// 	return;
// }

// template< T, C >
// void Buffer::bindAttribute(Program* program, const std::string& attributeName, bool normalized) 
// {
// 	GLint attributeLocation = program->getAttributeLocation(attributeName);
// 	bindAttribute(program, attributeLocation, normalized);
// }





// template< T, C >
// void Buffer::unbindAttribute(Program* program, GLint attributeLocation) 
// {
// 	(void) program;
// 	glDisableVertexAttribArray(attributeLocation);
// 	glBindBuffer(GL_ARRAY_BUFFER, 0);
// 	SDL_assert(glGetError() == GL_NO_ERROR);
// }

// template< T, C >
// void Buffer::unbindAttribute(Program* program, const std::string& attributeName) 
// {
// 	GLint attributeLocation = program->getAttributeLocation(attributeName);
// 	unbindAttribute(program, attributeLocation);
// }



