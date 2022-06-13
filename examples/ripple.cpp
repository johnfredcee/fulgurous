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

#include "arraybuilder.h"
#include "buffer.h"
#include "bufferbuilder.h"
#include "context.h"
#include "gl_funcalls.h"
#include "gl_typetraits.h"
#include "shaderprogram.h"
#include "vec.h"
#include <shader.h>

namespace ripple
{

	static constexpr int NUM_X = 40; // total quads on X axis
	static constexpr int NUM_Z = 40; // total quads on Z axis

	static constexpr float SIZE_X = 4; // size of plane in world space
	static constexpr float SIZE_Z = 4;
	static constexpr float HALF_SIZE_X = SIZE_X / 2.0f;
	static constexpr float HALF_SIZE_Z = SIZE_Z / 2.0f;

	// ripple displacement speed
	static constexpr float SPEED = 2;

	static constexpr int TOTAL_INDICES = NUM_X * NUM_Z * 2 * 3;

} // namespace ripple

GLuint Context::width = 800;
GLuint Context::height = 600;

// Function prototypes
void key_cb(GLFWwindow *window, int key, int scancode, int action, int mode);
void error_cb(int error, const char *desc);
void fb_size_cb(GLFWwindow *window, int width, int height);
void mouse_button_cb(GLFWwindow *window, int button, int action, int mods);
void mouse_move_cb(GLFWwindow *window, double xpos, double ypos);

// Window dimensions
static constexpr GLuint WIDTH = 800, HEIGHT = 600;

struct RippleVertex
{
	Point3 position;
};

// projection and modelview matrices
Matrix4 proj(Matrix4::identity());
Matrix4 modelView(Matrix4::identity());

// camera transformation variables
int state = 0;
float oldX = 0, oldY = 0;
float rX = 25, rY = -40, camdist = -7;

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

using Vec4 = Vec<GLfloat, 4>;
using Vec3 = Vec<GLfloat, 3>;
using Index = Vec<GLushort, 1>;

GLuint vaoBuildID;
GLuint vboVerticesID;
GLuint vboIndicesID;
Matrix4 P = Matrix4::identity();

int width, height;

using namespace ripple;

// The MAIN function, from here we start the application and run the game loop
int main()
{

	std::unique_ptr<Context> context = std::make_unique<Context>(WIDTH, HEIGHT, "Ripple", false);
	{

		// Set the required callback functions
		context->setGLFWCallback(key_cb);
		context->setGLFWCallback(fb_size_cb);
		context->setGLFWCallback(mouse_button_cb);
		context->setGLFWCallback(mouse_move_cb);

		{
			std::shared_ptr<ShaderProgram> ripple_program(new ShaderProgram());
			ripple_program->load_from_file(ShaderKind::eVERTEX_SHADER, "./shaders/ripple.vert");
			ripple_program->load_from_file(ShaderKind::eFRAGMENT_SHADER, "./shaders/ripple.frag");
			ripple_program->compile(ShaderKind::eVERTEX_SHADER);
			ripple_program->compile(ShaderKind::eFRAGMENT_SHADER);
			ripple_program->link();
			ripple_program->use();

			for (auto &&uniform : ripple_program->uniforms)
			{
				std::cout << "Uniform " << uniform.location << " : " << uniform.name << std::endl;
			}

			for (auto &&attribute : ripple_program->attributes)
			{
				std::cout << "Attribute " << attribute.location << " : " << attribute.name << std::endl;
			}
			ripple_program->unuse();

			BufferBuilder<Vec3> ripple_positions;
			BufferBuilder<Index> ripple_indices;

			// setup plane geometry
			// setup plane vertices
			int count = 0;
			int i = 0, j = 0;
			for (j = 0; j <= NUM_Z; j++)
			{
				for (i = 0; i <= NUM_X; i++)
				{
					ripple_positions.emplace(((float(i) / (NUM_X - 1)) * 2 - 1) * HALF_SIZE_X, 0.0f, ((float(j) / (NUM_Z - 1)) * 2 - 1) * HALF_SIZE_Z);
				}
			}

			// fill plane indices array
			for (i = 0; i < NUM_Z; i++)
			{
				for (j = 0; j < NUM_X; j++)
				{
					int i0 = i * (NUM_X + 1) + j;
					int i1 = i0 + 1;
					int i2 = i0 + (NUM_X + 1);
					int i3 = i2 + 1;
					if ((j + i) % 2)
					{
						ripple_indices.emplace(i0);
						ripple_indices.emplace(i2);
						ripple_indices.emplace(i1);
						ripple_indices.emplace(i1);
						ripple_indices.emplace(i2);
						ripple_indices.emplace(i3);
					}
					else
					{
						ripple_indices.emplace(i0);
						ripple_indices.emplace(i2);
						ripple_indices.emplace(i3);
						ripple_indices.emplace(i0);
						ripple_indices.emplace(i3);
						ripple_indices.emplace(i1);
					}
				}
			}

			array_builder(vaoBuildID,
						  ripple_program,
						  BufferInitialiser<Vec3>{"vVertex", ripple_positions, GL_ARRAY_BUFFER, GL_STATIC_DRAW},
						  BufferInitialiser<Index>{"", ripple_indices, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW});

			context->drawcb = [](const Context &context, float alpha)
			{
				// Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
				glfwPollEvents();

				const auto [mx, my] = context.getCursorPos();

				// Clear the colorbuffer
				gl_exec(glClearColor, 0.2f, 0.3f, 0.3f, 1.0f);
				gl_exec(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				float vtime = (float)glfwGetTime() / 1000.0f * SPEED;

				Transform3 MV = Transform3::translation(Vector3(0.0f, 0.0f, camdist)) * Transform3::rotationX(rX) * Transform3::rotationY(rY);
				Matrix4 MVP = P * MV;

				std::shared_ptr<ShaderProgram> ripple_program(context.programs[0]);
				ripple_program->use();
				gl_exec(glBindVertexArray, vaoBuildID);
				GLint location = ripple_program->uniform_location("MVP");
				auto mvp = glMat4(MVP);
				gl_exec(glUniformMatrix4fv, location, 1, GL_FALSE, mvp.get());
				location = ripple_program->uniform_location("time");
				gl_exec(glUniform1f, location, vtime);
				gl_exec(glDrawElements, GL_TRIANGLES, TOTAL_INDICES, GL_UNSIGNED_SHORT, nullptr);
				gl_exec(glBindVertexArray, 0);
				ripple_program->unuse();
			};

			// Game loop
			while (!context->done())
			{
				context->draw();
			}
		}
		context.release();
		// Terminates GLFW, clearing any resources allocated by GLFW.
		glfwTerminate();
		return 0;
	}
}
// Is called whenever a key is pressed/released via GLFW
void key_cb(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	std::cout << key << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void fb_size_cb(GLFWwindow *window, int fbwidth, int fbheight)
{
	width = fbwidth;
	height = fbheight;
	glViewport(0, 0, width, height);
	P = Matrix4::perspective(3.14f / 4.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void error_cb(int error, const char *desc)
{
	std::cerr << "GLFW error " << error << desc << std::endl;
}

double mouseXPos, mouseYPos;

void mouse_button_cb(GLFWwindow *window, int button, int action, int mods)
{
	if ((button = GLFW_MOUSE_BUTTON_LEFT) && (action == GLFW_PRESS))
	{
		oldX = (float)mouseXPos;
		oldY = (float)mouseYPos;
	}

	if ((button = GLFW_MOUSE_BUTTON_RIGHT))
	{
		state = 0;
	}
	else
	{
		state = 1;
	}
}

void mouse_move_cb(GLFWwindow *window, double xpos, double ypos)
{
	mouseXPos = xpos;
	mouseYPos = ypos;
	if (state == 0)
	{
		camdist *= (1 + ((float)(float)ypos - oldY) / 60.0f);
	}
	else
	{
		rY += ((float)xpos - oldX) / 5.0f;
		rX += ((float)ypos - oldY) / 5.0f;
	}
	oldX = (float)xpos;
	oldY = (float)ypos;
}
