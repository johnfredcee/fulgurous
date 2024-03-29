#pragma once
#include <variant>

using Callback = std::variant<GLFWerrorfun, GLFWframebuffersizefun, GLFWkeyfun, GLFWmousebuttonfun, GLFWcursorposfun>;

struct Context
{


	// Window dimensions
	static GLuint width;
	static GLuint height;

	// callbcak types
	GLFWerrorfun 	        errorcb;
	GLFWkeyfun 			    keycb;
	GLFWframebuffersizefun  fbsizecb;

	// shader programs
	std::vector<std::shared_ptr<ShaderProgram>> programs;

	// drawing contexts
	NVGcontext *vg;
	GLFWwindow *window;


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

	// the "overload" pattern: https://www.bfilipek.com/2019/02/2lines3featuresoverload.html 
	template <class... Ts>
	struct overloaded : Ts...
	{
		using Ts::operator()...;
	};
	template <class... Ts>
	overloaded(Ts...)->overloaded<Ts...>;

	void setGLFWCallback(Callback callbackfn)
	{
		std::visit(overloaded{
					   [this](GLFWerrorfun fn) { glfwSetErrorCallback(fn); },
					   [this](GLFWframebuffersizefun fn) { glfwSetFramebufferSizeCallback(window, fn); },
					   [this](GLFWkeyfun fn) { glfwSetKeyCallback(window, fn); },
					   [this](GLFWmousebuttonfun fn) { glfwSetMouseButtonCallback(window, fn); },
					   [this](GLFWcursorposfun fn) { glfwSetCursorPosCallback(window, fn); },
				   },
				   callbackfn);
	}

	Context(GLuint width, GLuint height, const char *title, bool resizable = false)
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
		glfwWindowHint(GLFW_RESIZABLE, resizable ? GL_TRUE : GL_FALSE);

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


	auto Context::getFrameBufferSize() const
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		return std::make_tuple(width, height);
	}

	auto Context::getCursorPos() const
	{
		double mx, my;
		glfwGetCursorPos(window, &mx, &my);
		return std::make_tuple(mx, my);
	}

	// context update functions
	typedef void (*updatefun)(const Context &context);
	typedef void (*drawfun)(const Context &context, float alpha);

	drawfun drawcb{[](const Context &context, float alpha) { return; }};
	updatefun updatecb{[](const Context &context) { return; }};

	~Context()
	{
		gl_exec(glUseProgram, 0);
		programs.clear();
		glfwSetKeyCallback(window, nullptr);
		glfwSetFramebufferSizeCallback(window, nullptr);
		nvgDeleteGL3(vg);
		glfwSetErrorCallback(nullptr);
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
