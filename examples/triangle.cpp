#include <initializer_list>
#include <iostream>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// THIS IS OPTIONAL AND NOT REQUIRED, ONLY USE THIS IF YOU DON'T WANT GLAD TO INCLUDE windows.h
// GLAD will include windows.h for APIENTRY if it was not previously defined.
// Make sure you have the correct definition for APIENTRY for platforms which define _WIN32 but don't use __stdcall
#ifdef _WIN32
#define APIENTRY __stdcall
#endif

// GLAD
#include <glad/glad.h>

// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
#error windows.h was included!
#endif

// GLFW
#include "nanovg.h"
#include <GLFW/glfw3.h>
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "vectormath_aos.h"

using namespace Vectormath;
using namespace Vectormath::Aos;

#include "filesystem/path.h"
#include "filesystem/resolver.h"

#include <vec.h>
#include <gl_funcalls.h>
#include <gl_typetraits.h>
#include <shader.h>
#include <shaderprogram.h>
#include "arraybuilder.h"
#include "buffer.h"
#include "bufferbuilder.h"
#include "context.h"
#include "drawcall.h"
#include "framebuffer.h"
#include "vertexdata.h"

GLuint Context::width = 800;
GLuint Context::height = 600;

struct ColouredVertex
{
	Point3 position;
	Vector4 colour;
};

void drawNVGWindow(NVGcontext *vg, const char *title, float x, float y, float w, float h)
{
	float cornerRadius = 3.0f;

	nvgSave(vg);
	//

	nvgBeginPath(vg);
	nvgRoundedRect(vg, x, y, w, h, cornerRadius);
	nvgFillColor(vg, nvgRGBA(28, 30, 34, 192));
	//	nvgFillColor(vg, nvgRGBA(0,0,0,128));
	nvgFill(vg);

	nvgFontSize(vg, 18.0f);
	nvgFontFace(vg, "sans-bold");
	nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

	nvgFontBlur(vg, 0.0f);
	nvgFillColor(vg, nvgRGBA(192, 192, 192, 128));
	nvgText(vg, x + w / 2, y + 16 + 1, title, NULL);

	nvgRestore(vg);
}

std::shared_ptr<float[]> glMat4(const Matrix4 &mat4)
{
	std::shared_ptr<float[]> result(new float[16]);
	float *result_ptr = result.get();
	storeXYZW(mat4.getCol0(), result_ptr);
	storeXYZW(mat4.getCol1(), &result_ptr[4]);
	storeXYZW(mat4.getCol2(), &result_ptr[8]);
	storeXYZW(mat4.getCol3(), &result_ptr[12]);
	return result;
}

ColouredVertex vertices[3];
GLshort indices[3];
GLuint vaoID;
GLuint vaoBuildID;
GLuint vboVerticesID;
GLuint vboColorsID;
GLuint vboIndicesID;
Matrix4 proj = Matrix4::orthographic(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f);
Matrix4 model_view = Matrix4::identity();

int ghing = GL_FLOAT;

int main()
{
	std::unique_ptr<Context> context = std::make_unique<Context>(800, 600, "Triangle");
	{
		std::shared_ptr<ShaderProgram> program(new ShaderProgram());
		program->load_from_file(ShaderKind::eVERTEX_SHADER, "./shaders/shader.vert");
		program->load_from_file(ShaderKind::eFRAGMENT_SHADER, "./shaders/shader.frag");
		program->compile(ShaderKind::eVERTEX_SHADER);
		program->compile(ShaderKind::eFRAGMENT_SHADER);
		program->link();
		program->use();

		for (auto &&uniform : program->uniforms)
		{
			std::cout << "Uniform " << uniform.location << " : " << uniform.name << std::endl;
		}

		for (auto &&attribute : program->attributes)
		{
			std::cout << "Attribute " << attribute.location << " : " << attribute.name << std::endl;
		}
		program->unuse();
		context->programs.push_back(program);
		using Vec4 = Vec<GLfloat, 4>;
		using Vec3 = Vec<GLfloat, 3>;
		using Index = Vec<GLushort, 1>;

		/* Buiid buffers */
		BufferBuilder<Vec3> positions = {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}};
		BufferBuilder<Vec4> colors = {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}};
		BufferBuilder<Index> indices = {{0}, {1}, {2}};

		/* Assign buffers to vao */
		array_builder(vaoBuildID,
					  program,
					  BufferInitialiser<Vec3>{"vVertex", positions, GL_ARRAY_BUFFER, GL_STATIC_DRAW},
					  BufferInitialiser<Vec4>{"vColor", colors, GL_ARRAY_BUFFER, GL_STATIC_DRAW},
					  IndexBufferInitialiser<Index>{indices, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW});

		context->drawcb = [](const Context &context, float alpha)
		{
			double mx, my;
			int winWidth, winHeight;
			int fbWidth, fbHeight;
			float pxRatio;
			glfwGetCursorPos(context.window, &mx, &my);
			glfwGetWindowSize(context.window, &winWidth, &winHeight);
			glfwGetFramebufferSize(context.window, &fbWidth, &fbHeight);
			// Calculate pixel ration for hi-dpi devices.
			pxRatio = (float)fbWidth / (float)winWidth;

			// Update and render
			gl_exec(glViewport, 0, 0, fbWidth, fbHeight);

			// Clear the colorbuffer
			gl_exec(glClearColor, 0.2f, 0.3f, 0.3f, 1.0f);
			gl_exec(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			// nvgEndFrame(vg);
			std::shared_ptr<ShaderProgram> program(context.programs[0]);
			program->use();
			glBindVertexArray(vaoBuildID);
			GLint location = program->uniform_location("MVP");
			Matrix4 modelview_projection = proj * model_view;
			std::shared_ptr<float[]> mvp = glMat4(modelview_projection);
			glUniformMatrix4fv(location, 1, GL_FALSE, mvp.get());
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
			glBindVertexArray(0);
			program->unuse();
		};

		// Game loop
		while (!context->done())
		{
			context->draw();
		}
		program->unuse();
	}
	// Terminates GLFW, clearing any resources allocated by GLFW.
	delete context.release();
	glfwTerminate();
	return 0;
}
