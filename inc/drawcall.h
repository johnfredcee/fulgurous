#pragma once
#include <iostream>
#include <memory>



struct DrawCall
{
	std::shared_ptr<ShaderProgram> program;
	GLuint mSize;
	GLenum mType;

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

	static std::shared_ptr<float[]> glMat3(const Matrix3 &mat3)
	{
		std::shared_ptr<float[]> result(new float[9]);
		float *result_ptr = result.get();
		storeXYZ(mat3.getCol0(), result_ptr);
		storeXYZ(mat3.getCol1(), &result_ptr[3]);
		storeXYZ(mat3.getCol2(), &result_ptr[6]);
		return result;
	}

	DrawCall() = delete;
	DrawCall(const DrawCall &other) = delete;
	DrawCall &operator=(const DrawCall &other) = delete;
	DrawCall(DrawCall &&other) = delete;
	DrawCall &operator=(DrawCall &&other) = delete;


	DrawCall(std::shared_ptr<ShaderProgram> inProgram) : program(inProgram)
	{	
		program->use();
	}

	template<typename T>
	void addBuffer(std::string name, std::shared_ptr< Buffer<T> > buffer)
	{
		buffer->bindAttribute(program, name);
	}

	template<typename T>
	void addIndexBuffer(std::shared_ptr< Buffer<T> > buffer)
	{
		buffer->bindIndices();
		mSize = buffer->getSize();
		mType = buffer->getType();
	}

	template<typename T>
	void addUniform(std::string uniformName, T &data);

	template<>
	void addUniform(std::string uniformName, Matrix4 &data)
	{
		GLuint location = program->uniform_location(uniformName);
	    std::shared_ptr<float[]> m4 = glMat4(data);    
		glUniformMatrix4fv(location, 1, GL_FALSE, m4.get());
	}

	template<>
	void addUniform(std::string uniformName, Matrix3 &data)
	{
		GLuint location = program->uniform_location(uniformName);
	    std::shared_ptr<float[]> m3 = glMat3(data);    
		glUniformMatrix3fv(location, 1, GL_FALSE, m3.get());
	}

	template<>
	void addUniform(std::string uniformName, GLfloat& v)
	{
		GLuint location = program->uniform_location(uniformName);
	  	glUniform1f(location, v);
	}

	template<>
	void addUniform(std::string uniformName, Point3& p)
	{
		static GLfloat v3[3];
		GLuint location = program->uniform_location(uniformName);
		storeXYZ(p, v3);
	  	glUniform3fv(location, 1, v3);
	}

	template<>
	void addUniform(std::string uniformName, Vector3& v)
	{
		static GLfloat v3[3];
		GLuint location = program->uniform_location(uniformName);
		storeXYZ(v, v3);
	  	glUniform3fv(location, 1, v3);
	}

	template<>
	void addUniform(std::string uniformName, Vec<GLfloat, 3>& v)
	{
		GLuint location = program->uniform_location(uniformName);
	  	glUniform3fv(location, 1, &v.x);
	}

	template<>
	void addUniform(std::string uniformName, Vector4& v)
	{
		static GLfloat v4[4];
		GLuint location = program->uniform_location(uniformName);
		storeXYZW(v, v4);
	  	glUniform4fv(location, 1, v4);
	}

	template<>
	void addUniform(std::string uniformName, Vec<GLfloat, 4>& v)
	{
		GLuint location = program->uniform_location(uniformName);
	  	glUniform4fv(location, 1, &v.x);
	}

	template<>
	void addUniform(std::string uniformName, Quat& q)
	{
		static GLfloat v4[4];
		GLuint location = program->uniform_location(uniformName);
		storeXYZW(q, v4);
	  	glUniform4fv(location, 1, v4);
	}

	void draw(GLenum mode = GL_TRIANGLES)
	{
		gl_exec(glDrawElements, mode, mSize, mType, (void*) 0);
	}

	~DrawCall()
	{
		for(auto& attribute : program->attributes)
		{
		   GLint location = program->attribute_location(attribute.name);
		   gl_exec(glDisableVertexAttribArray, location);
		}
		program->unuse();

	}
};