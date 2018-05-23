#include <iostream>
#include <memory>

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

void errorcb(int error, const char* desc)
{
	std::cerr << "GLFW error " << error << desc << std::endl;
}


// This example is taken from http://learnopengl.com/
// http://learnopengl.com/code_viewer.php?code=getting-started/hellowindow2
// The code originally used GLEW, I replaced it with Glad

// Compile:
// g++ example/c++/hellowindow2.cpp -Ibuild/include build/src/glad.c -lglfw -ldl


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

void drawWindow(NVGcontext* vg, const char* title, float x, float y, float w, float h)
{
   	float cornerRadius = 3.0f;

    nvgSave(vg);
    //	

   	nvgBeginPath(vg);
	nvgRoundedRect(vg, x,y, w,h, cornerRadius);
	nvgFillColor(vg, nvgRGBA(28,30,34,192));
//	nvgFillColor(vg, nvgRGBA(0,0,0,128));
	nvgFill(vg);

    nvgFontSize(vg, 18.0f);
	nvgFontFace(vg, "sans-bold");
	nvgTextAlign(vg,NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);

	nvgFontBlur(vg,0.0f);
	nvgFillColor(vg, nvgRGBA(192,192,192,128));
	nvgText(vg, x+w/2,y+16+1, title, NULL);

	nvgRestore(vg);
}

std::shared_ptr<float[]> glMat4(const Matrix4& mat4)
{
    std::shared_ptr<float[]> result(new float[16]);
    float* result_ptr = result.get();
    storeXYZW(mat4.getCol0(), result_ptr);
    storeXYZW(mat4.getCol1(), &result_ptr[4]);
    storeXYZW(mat4.getCol2(), &result_ptr[8]);
    storeXYZW(mat4.getCol3(), &result_ptr[12]);
    return result;
}
    
// The MAIN function, from here we start the application and run the game loop
int main()
{
	NVGcontext* vg = NULL;

    std::cout << "Starting GLFW context, OpenGL 4.1" << std::endl;
    // Init GLFW
    glfwInit();

    // Handle errors
	glfwSetErrorCallback(errorcb);
    
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

	vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);

	int fontBold = nvgCreateFont(vg, "sans-bold", "./nanovg/example//Roboto-Bold.ttf");
	if (fontBold == -1) {
		std::cerr << "Could not add font bold.\n" << std::endl;
		return -1;
	}

	glfwSwapInterval(0);

    GLuint vertShader = glCreateShader( GL_VERTEX_SHADER );
    if( 0 == vertShader )
    {
        std::cerr << "Error creating vertex shader." << std::endl;
    }

    // Define the viewport dimensions
    glViewport(0, 0, WIDTH, HEIGHT);

    Point3 position{ 1.0f, 1.0f, - 1.0f };
    Point3 eye_pos{0.0f, 0.0f, 5.0f};
    Point3 lookat_pos{0.0f, 0.0f, 0.0f};
    Vector3 up{0.0f, 0.0f, 1.0f};
    Matrix4 view(Matrix4::lookAt(eye_pos, lookat_pos, up));
    Matrix4 model = Matrix4::identity();
    model *= Matrix4::rotation(3.145f / 2.0f, up);
    Matrix4 model_view = view * model;
    Vector4 transformed = model_view * position;
    std::shared_ptr<float[]> mvp = glMat4(model_view);

    
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        double mx, my, t, dt;
		int winWidth, winHeight;
		int fbWidth, fbHeight;
		float pxRatio;

        // Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();


		glfwGetCursorPos(window, &mx, &my);
		glfwGetWindowSize(window, &winWidth, &winHeight);
		glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
		// Calculate pixel ration for hi-dpi devices.
		pxRatio = (float)fbWidth / (float)winWidth;

   		// Update and render
		glViewport(0, 0, fbWidth, fbHeight);

        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

		nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

    	drawWindow(vg, "Widgets `n Stuff", 50, 50, 300, 400);
    
    	nvgEndFrame(vg);

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

	nvgDeleteGL3(vg);

    // Terminates GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    std::cout << key << std::endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}