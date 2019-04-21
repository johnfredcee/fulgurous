#include <iostream>
#include <memory>
#include <vector>
#include <tuple>
#include <type_traits>
#include <initializer_list>
#include <utility>

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
#include <GLFW/glfw3.h>
#include "nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "vectormath_aos.h"

using namespace Vectormath;
using namespace Vectormath::Aos;

#include "filesystem/path.h"
#include "filesystem/resolver.h"

#include "shader.h"
#include "gl_typetraits.h"
#include "gl_funcalls.h"
#include "vec.h"
#include "buffer.h"
#include "bufferbuilder.h"
#include "arraybuilder.h"

namespace ripple
{

static constexpr int NUM_X = 40; //total quads on X axis
static constexpr int NUM_Z = 40; //total quads on Z axis

static constexpr float SIZE_X = 4; //size of plane in world space
static constexpr float SIZE_Z = 4;
static constexpr float HALF_SIZE_X = SIZE_X / 2.0f;
static constexpr float HALF_SIZE_Z = SIZE_Z / 2.0f;

//ripple displacement speed
static constexpr float SPEED = 2;

static constexpr int TOTAL_INDICES = NUM_X * NUM_Z * 2 * 3;

} // namespace ripple

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

//projection and modelview matrices
Matrix4 proj(Matrix4::identity());
Matrix4 modelView(Matrix4::identity());

//camera transformation variables
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

GLuint vaoBuildID;
GLuint vboVerticesID;
GLuint vboIndicesID;
Matrix4 P = Matrix4::identity();

int width, height;

//current time
float time = 0;

using namespace ripple;

// The MAIN function, from here we start the application and run the game loop
int main()
{

	std::cout << "Starting GLFW corecontext, OpenGL 3.3" << std::endl;
	// Init GLFW
	glfwInit();

	// Handle errors
	glfwSetErrorCallback(error_cb);

	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Ripple", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	// Set the required callback functions
	glfwSetKeyCallback(window, key_cb);
	glfwSetFramebufferSizeCallback(window, fb_size_cb);
	glfwSetMouseButtonCallback(window, mouse_button_cb);
	glfwSetCursorPosCallback(window, mouse_move_cb);	
	
    glfwGetFramebufferSize(window, &width, &height);
    fb_size_cb(window, width, height);

	glfwSwapInterval(0);

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
		BufferBuilder<Vec<GLushort, 1>> ripple_indices;

		//setup plane geometry
		//setup plane vertices
		int count = 0;
		int i = 0, j = 0;
		for (j = 0; j <= NUM_Z; j++)
		{
			for (i = 0; i <= NUM_X; i++)
			{
				ripple_positions.emplace(((float(i) / (NUM_X - 1)) * 2 - 1) * HALF_SIZE_X, 0.0f, ((float(j) / (NUM_Z - 1)) * 2 - 1) * HALF_SIZE_Z);
			}
		}

		//fill plane indices array
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
					 BufferInitialiser<Vec<GLushort, 1>>{"", ripple_indices, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW});

		// Game loop
		while (!glfwWindowShouldClose(window))
		{
			double mx, my, t, dt;
			float pxRatio;

			// Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
			glfwPollEvents();

			glfwGetCursorPos(window, &mx, &my);
			
			// Clear the colorbuffer
			gl_exec(glClearColor, 0.2f, 0.3f, 0.3f, 1.0f);
			gl_exec(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			float vtime = glfwGetTime() / 1000.0f * SPEED;

			Matrix4 MV = Matrix4::rotationY(rY) * Matrix4::rotationX(rX) * Matrix4::translation(Vector3(0.0f, 0.0f, camdist));
			Matrix4 MVP = P * MV;

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
			// Swap the screen buffers
			glfwSwapBuffers(window);
		}
	}
	// Terminates GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
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
	P = Matrix4::perspective(3.14f / 4.0f, (GLfloat)width / (GLfloat)  height, 0.1f, 100.0f);
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
		oldX = mouseXPos;
		oldY = mouseYPos;
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
		camdist *= (1 + (ypos - oldY) / 60.0f);
	}
	else
	{
		rY += (xpos - oldX) / 5.0f;
		rX += (ypos - oldY) / 5.0f;
	}
	oldX = xpos;
	oldY = ypos;
}
