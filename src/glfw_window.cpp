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

using namespace ripple;

// The MAIN function, from here we start the application and run the game loop
int main()
{
    NVGcontext *vg = NULL;

    std::cout << "Starting GLFW context, OpenGL 4.1" << std::endl;
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
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", NULL, NULL);
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

        using Vec4 = Vec<GLfloat, 4>;
        using Vec3 = Vec<GLfloat, 3>;

        BufferBuilder<Vec3> positions = {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}};
        BufferBuilder<Vec4> colors = {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}};
        BufferBuilder<Vec<GLushort, 1>> indices = {{0}, {1}, {2}};

        arrayBuilder(vaoBuildID,
                     program,
                     BufferInitialiser<Vec3>{"vVertex", positions, GL_ARRAY_BUFFER, GL_STATIC_DRAW},
                     BufferInitialiser<Vec4>{"vColor", colors, GL_ARRAY_BUFFER, GL_STATIC_DRAW},
                     BufferInitialiser<Vec<GLushort, 1>>{"", indices, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW});

#if 0
        GLsizei stride = sizeof(ColouredVertex);
        gl_exec(glGenVertexArrays, 1, &vaoID);
    
    gl_exec(glGenBuffers, 1, &vboVerticesID);
    gl_exec(glGenBuffers, 1, &vboColorsID);
    gl_exec(glGenBuffers, 1, &vboIndicesID);
        gl_exec(glBindVertexArray, vaoID);

        std::shared_ptr<Buffer<Vec3>> glbVertices = positions.make_buffer(GL_ARRAY_BUFFER,GL_STATIC_DRAW);
        //produce_buffer<Vec<GLfloat, 3>>(GL_ARRAY_BUFFER, positions.getData(), positions.elementCount(), GL_STATIC_DRAW);

        // positions.make_buffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
        // gl_exec(glBindBuffer, GL_ARRAY_BUFFER, vboVerticesID);
        // gl_exec(glBufferData, GL_ARRAY_BUFFER, positions.byteSize(), positions.getData(), GL_STATIC_DRAW);
        glbVertices->bindAttribute(program, "vVertex");

        // GLint location = program->attribute_location("vVertex");
        // gl_exec(glEnableVertexAttribArray, location);
        // gl_exec(glVertexAttribPointer, location, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        std::shared_ptr<Buffer<Vec4>> glbColors = colors.make_buffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
        //smake_buffer<Vec<GLfloat, 4>>(GL_ARRAY_BUFFER, colors.getData(), colors.elementCount(), GL_STATIC_DRAW);
        // gl_exec(glBindBuffer, GL_ARRAY_BUFFER, vboColorsID);
        // gl_exec(glBufferData, GL_ARRAY_BUFFER, colors.byteSize(), colors.getData(), GL_STATIC_DRAW);
        // GLint location = program->attribute_location("vColor");
        glbColors->bindAttribute(program, "vColor");
        // location = program->attribute_location("vColor");
        // gl_exec(glEnableVertexAttribArray, location);
        // gl_exec(glVertexAttribPointer, location, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

        std::shared_ptr<Buffer<Vec<GLushort,1>>> glbIndices = indices.make_buffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
        //produce_buffer<Vec<GLushort, 1>>(GL_ELEMENT_ARRAY_BUFFER, indices.getData(), indices.elementCount(), GL_STATIC_DRAW);
        //glbIndices->bindIndices();

        // gl_exec(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
        // gl_exec(glBufferData, GL_ELEMENT_ARRAY_BUFFER, indices.byteSize(), indices.getData(), GL_STATIC_DRAW);

        gl_exec(glBindVertexArray, 0);
#endif
        std::shared_ptr<ShaderProgram> ripple_program(new ShaderProgram());
        ripple_program->load_from_file(ShaderKind::eVERTEX_SHADER, "./shaders/shader.vert");
        ripple_program->load_from_file(ShaderKind::eFRAGMENT_SHADER, "./shaders/shader.frag");
        ripple_program->compile(ShaderKind::eVERTEX_SHADER);
        ripple_program->compile(ShaderKind::eFRAGMENT_SHADER);
        ripple_program->link();
        ripple_program->use();

        BufferBuilder<Vec3> ripple_positions;
        BufferBuilder<Vec<GLushort, 1>> ripple_indices;

        int count = 0;
        int i = 0, j = 0;
        for (j = 0; j <= NUM_Z; j++)
        {
            for (i = 0; i <= NUM_X; i++)
            {
                ripple_positions.add(Vec3{((float(i) / (NUM_X - 1.0f)) * 2.0f - 1.0f) * HALF_SIZE_X, 0.0f, ((float(j) / (NUM_Z - 1.0f)) * 2.0f - 1.0f) * HALF_SIZE_Z});
            }
        }

        //fill plane indices array
        for (i = 0; i < NUM_Z; i++)
        {
            for (j = 0; j < NUM_X; j++)
            {
                GLushort i0 = i * (NUM_X + 1) + j;
                GLushort i1 = i0 + 1;
                GLushort i2 = i0 + (NUM_X + 1);
                GLushort i3 = i2 + 1;
                if ((j + i) % 2)
                {
                    ripple_indices.add(Vec<GLushort,1>{i0});
                    ripple_indices.add(Vec<GLushort,1>{i2});
                    ripple_indices.add(Vec<GLushort,1>{i1});
                    ripple_indices.add(Vec<GLushort,1>{i1});
                    ripple_indices.add(Vec<GLushort,1>{i2});
                    ripple_indices.add(Vec<GLushort,1>{i3});
                }
                else
                {
                    ripple_indices.add(Vec<GLushort,1>{i0});
                    ripple_indices.add(Vec<GLushort,1>{i2});
                    ripple_indices.add(Vec<GLushort,1>{i3});
                    ripple_indices.add(Vec<GLushort,1>{i0});
                    ripple_indices.add(Vec<GLushort,1>{i3});
                    ripple_indices.add(Vec<GLushort,1>{i1});
                }
            }
        }

        // Define the viewport dimensions
        // glViewport(0, 0, WIDTH, HEIGHT);

        // Point3 position{ 1.0f, 1.0f, - 1.0f };
        // Point3 eye_pos{0.0f, 0.0f, 5.0f};
        // Point3 lookat_pos{0.0f, 0.0f, 0.0f};
        // Vector3 up{0.0f, 0.0f, 1.0f};
        // Matrix4 view(Matrix4::lookAt(eye_pos, lookat_pos, up));
        // Matrix4 model = Matrix4::identity();
        // model *= Matrix4::rotation(3.145f / 2.0f, up);
        // Matrix4 model_view = view * model;
        Matrix4 model_view = Matrix4::identity();
        // Vector4 transformed = model_view * position;
        // std::shared_ptr<float[]> mvp = glMat4(model_view);

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