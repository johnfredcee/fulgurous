#pragma once

struct Context
{

	// Window dimensions
	static GLuint width;
	static GLuint height;
	NVGcontext *vg;
	GLFWerrorfun errorcb;
	GLFWkeyfun keycb;
	GLFWframebuffersizefun fbsizecn;
	GLFWwindow *window;
	typedef void (*updatefun)(const Context &context);
	typedef void (*drawfun)(const Context &context, float alpha);
	drawfun drawcb{[](const Context &context, float alpha) { return; }};
	updatefun updatecb{[](const Context &context) { return; }};
	std::vector<std::shared_ptr<ShaderProgram>> programs;

	static void default_error_cb(int error, const char *desc)
	{
		std::cerr << "GLFW error " << error << desc << std::endl;
	}

	// Is called whenever a key is pressed/released via GLFW
	static void default_key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
	{
		std::cout << key << std::endl;
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
	}

	static void default_fbsize_callback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	Context(GLuint width, GLuint height, const char *title)
	{
		vg = nullptr;
		this->width = width;
		this->height = height;
		std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
		// Init GLFW
		glfwInit();
		errorcb = default_error_cb;
		glfwSetErrorCallback(default_error_cb);
		// Set all the required options for GLFW
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

		// Create a GLFWwindow object that we can use for GLFW's functions
		window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		glfwMakeContextCurrent(window);
		if (window == nullptr)
		{
			std::cerr << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
		}

		// Set the required callback functions
		glfwSetKeyCallback(window, default_key_callback);
		glfwSetFramebufferSizeCallback(window, default_fbsize_callback);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cerr << "Failed to initialize OpenGL context" << std::endl;
			glfwTerminate();
		}

		vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
		int fontBold = nvgCreateFont(vg, "sans-bold", "./nanovg/example//Roboto-Bold.ttf");
		if (fontBold == -1)
		{
			std::cerr << "Could not add font bold.\n"
					  << std::endl;
			glfwTerminate();
		}
		glfwSwapInterval(0);
	}

	~Context()
	{
		glfwSetKeyCallback(window, nullptr);
		glfwSetFramebufferSizeCallback(window, nullptr);
		nvgDeleteGL3(vg);
		glfwSetErrorCallback(nullptr);
		// Terminates GLFW, clearing any resources allocated by GLFW.
		glfwTerminate();
	}

	bool done()
	{
		return glfwWindowShouldClose(window);
	}

	void draw()
	{
		// Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		drawcb(*this, 0.0f);
		// Swap the screen buffers
		glfwSwapBuffers(window);
		return;
	}

	Context() = delete;
	Context(const Context &other) = delete;
	Context &operator=(const Context &other) = delete;
	Context(Context &&other) = delete;
	Context &operator=(Context &&other) = delete;
};
