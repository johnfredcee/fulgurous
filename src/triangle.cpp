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

void errorcb(int error, const char *desc)
{
    std::cerr << "GLFW error " << error << desc << std::endl;
}

// This example is taken from http://learnopengl.com/
// http://learnopengl.com/code_viewer.php?code=getting-started/hellowindow2
// The code originally used GLEW, I replaced it with Glad

// Compile:
// g++ example/c++/hellowindow2.cpp -Ibuild/include build/src/glad.c -lglfw -ldl

// Function prototypes
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

void fb_size_callback(GLFWwindow *window, int width, int height);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

struct ColouredVertex
{
    Point3 position;
    Vector4 colour;
};

void drawWindow(NVGcontext *vg, const char *title, float x, float y, float w, float h)
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
Matrix4 proj = Matrix4::identity();

int ghing = GL_FLOAT;

// The MAIN function, from here we start the application and run the game loop
int main()
{
    NVGcontext *vg = NULL;

    std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
    // Init GLFW
    glfwInit();

    // Handle errors
    glfwSetErrorCallback(errorcb);

    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Triangle", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, fb_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    {
        int fontBold = nvgCreateFont(vg, "sans-bold", "./nanovg/example//Roboto-Bold.ttf");
        if (fontBold == -1)
        {
            std::cerr << "Could not add font bold.\n"
                      << std::endl;
            return -1;
        }

        glfwSwapInterval(0);

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

        using Vec4 = Vec<GLfloat,4>;
        using Vec3 = Vec<GLfloat,3>;

        BufferBuilder<Vec3> positions = {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}};
        BufferBuilder<Vec4> colors = {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}};
        BufferBuilder<Vec<GLushort,1>> indices = {{0}, {1}, {2}};

        arrayBuilder(vaoBuildID,
                    program,
                    BufferInitialiser<Vec3>{"vVertex", positions, GL_ARRAY_BUFFER, GL_STATIC_DRAW},
                    BufferInitialiser<Vec4>{"vColor", colors, GL_ARRAY_BUFFER, GL_STATIC_DRAW},
                    BufferInitialiser<Vec<GLushort,1>>{"", indices, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW});

        Matrix4 model_view = Matrix4::identity();

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
            gl_exec(glViewport, 0, 0, fbWidth, fbHeight);

            // Clear the colorbuffer
            gl_exec(glClearColor, 0.2f, 0.3f, 0.3f, 1.0f);
            gl_exec(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            // nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

            // drawWindow(vg, "Widgets `n Stuff", 50, 50, 300, 400);

            // nvgEndFrame(vg);

            program->use();
            glBindVertexArray(vaoBuildID);
            GLint location = program->uniform_location("MVP");
            Matrix4 modelview_projection = proj * model_view;
            std::shared_ptr<float[]> mvp = glMat4(modelview_projection);
            glUniformMatrix4fv(location, 1, GL_FALSE, mvp.get());
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 0);
            glBindVertexArray(0);
            program->unuse();
            // Swap the screen buffers
            glfwSwapBuffers(window);
        }
    }

    nvgDeleteGL3(vg);

    // Terminates GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    std::cout << key << std::endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void fb_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    proj = Matrix4::orthographic(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f);
}