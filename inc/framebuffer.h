#pragma once

class Framebuffer
{
	GLuint fbo;
public:
	Framebuffer()
	{
		glGenFramebuffers(1, &fbo);
	};

	void activate()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);  
	}

	void deactivate()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);  
	}
	
	~Framebuffer()
	{
		glDeleteFramebuffers(1, &fbo);
	}
};
