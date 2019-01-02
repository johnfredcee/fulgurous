#pragma once
#include <iostream>
#include <memory>


      
struct DrawCall
{
	std::shared_ptr<ShaderProgram> program;
	GLuint vaoID;

	static std::shared_ptr<float[]> glMat4(const Matrix4 &mat4)
	{
		std::shared_ptr<float[]> result(new float[16]);
		float *result_ptr = result.get();
		storeXYZW(mat4.getCol0(), result_ptr);
		storeXYZW(mat4.getCol1(), &result_ptr[4]);
		storeXYZW(mat4.getCol2(), &result_ptr[8]);
		storeXYZW(mat4.getCol3(), &result_ptr[12]);
		return result;
	}

	DrawCall() = delete;
	DrawCall(const DrawCall &other) = delete;
	DrawCall &operator=(const DrawCall &other) = delete;
	DrawCall(DrawCall &&other) = delete;
	DrawCall &operator=(DrawCall &&other) = delete;


	DrawCall(std::shared_ptr<ShaderProgram> inProgram) : program(inProgram)
	{
		gl_exec(glGenVertexArrays, 1, &vaoID);
	}

	void setupBuffers()
	{
		program->unuse();
		gl_exec(glBindVertexArray, vaoID);
	}

	template <typename T>
	void addBuffer(std::string attributeName, std::shared_ptr<BufferBuilder<T>> bufferBuilder, GLenum target, GLenum usage = GL_STATIC_DRAW)
	{
		auto buffer = bufferBuilder->make_buffer(target, usage);
		if (!parameterName.empty())
		{
			buffer->bindAttribute(program, parameterName);
		}
		else
		{
			buffer->bindIndices();
		}
	}
	
	void finishBuffers()
	{
		gl_exec(glBindVertexArray, vaoID);
	}

	void begin()
	{
		program->use();
		gl_exec(glBindVertexArray, vaoID);
	}

	void addUniform(std::string uniformName, Matrix4 &data)
	{
		GLuint location = program->uniform_location(uniformName);
	    std::shared_ptr<float[]> m4 = glMat4(data);    
		glUniformMatrix4fv(location, 1, GL_FALSE, m4.get());

	}

	void end()
	{
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
		glBindVertexArray(0);
		program->unuse();
	}
};