#include "ChokoLait.h"

//#define USE_DEBUG_CONTEXT
//#define SIMULATE_FRAMESCALE 2

#ifdef USE_DEBUG_CONTEXT
void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	void *userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		__debugbreak(); 
		std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;

	std::cout << std::endl;
}
#endif

GLFWwindow* ChokoLait::window = nullptr;
int ChokoLait::initd = 0;
rCamera ChokoLait::mainCamera = rCamera();
std::vector<dropFileFunc> ChokoLait::dropFuncs;
std::vector<emptyCallbackFunc> ChokoLait::focusFuncs;

#define _CHOKOLAIT_MAGIC 0x30cd84
uint CHOKOLAIT_MAGIC = _CHOKOLAIT_MAGIC;

void _printstack() {
	Debug::Error("System", "Dumping stack trace...");
	void* buf[50];
	uint c = Debug::StackTrace(50, buf);
#ifndef PLATFORM_WIN
	auto bs = backtrace_symbols(buf, c);
	for (uint a = 0; a < c; a++) {
		Debug::Error("System", bs[a]);
	}
#endif
}

void _dieded(int i) {
	if (CHOKOLAIT_MAGIC != _CHOKOLAIT_MAGIC) exit(-2);
	CHOKOLAIT_MAGIC = 0;
	Debug::Error("System", "Abort trap!");
	_printstack();
	exit(-1);	
}
void _sigfpe(int i) {
	if (CHOKOLAIT_MAGIC != _CHOKOLAIT_MAGIC) exit(-2);
	CHOKOLAIT_MAGIC = 0;
	Debug::Error("System", "FP exception!");
	_printstack();
	exit(-1);
}
void _sigseg(int i) {
	if (CHOKOLAIT_MAGIC != _CHOKOLAIT_MAGIC) exit(-2);
	CHOKOLAIT_MAGIC = 0;
	Debug::Error("System", "Segmentation fault!");
	_printstack();
	exit(-1);
}
void _sigtrm(int i) {
	if (CHOKOLAIT_MAGIC != _CHOKOLAIT_MAGIC) exit(-2);
	CHOKOLAIT_MAGIC = 0;
	Debug::Error("System", "Termination request!");
	_printstack();
	exit(-1);
}

void ChokoLait::_InitVars() {
#ifdef PLATFORM_WIN
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#else

#endif
	signal(SIGABRT, &_dieded);
	signal(SIGFPE, &_sigfpe);
	signal(SIGSEGV, &_sigseg);
	signal(SIGTERM, &_sigtrm);
	IO::InitPath();
	Debug::Init();
	Debug::Message("IO", "Path is " + IO::path);

	Debug::Message("System", "Opening GL context");
	if (!glfwInit()) {
		Debug::Error("System", "Fatal: Cannot init glfw!");
		abort();
	}
	glfwWindowHint(GLFW_VISIBLE, 0);

#ifdef USE_DEBUG_CONTEXT
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef USE_DEBUG_CONTEXT
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
	window = glfwCreateWindow(10, 10, "AViS " APPVERSION, NULL, NULL);
	Display::window = window;
	if (!window) {
		Debug::Error("System", "Fatal: Cannot create glfw window!");
		abort();
	}
	glfwMakeContextCurrent(window);


	glewExperimental = true;
	GLint GlewInitResult = glewInit();
	if (GLEW_OK != GlewInitResult)
	{
		Debug::Error("System", "Glew error: " + std::string((const char*)glewGetErrorString(GlewInitResult)));
		abort();
	}

#ifdef USE_DEBUG_CONTEXT
	GLint flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif
	
	Engine::Init();
}

void ChokoLait::Init(int scrW, int scrH) {
	if (!initd) {
		ChokoLait();
	}
	if (initd == 1) {
		Engine::_mainThreadId = std::this_thread::get_id();

		Scene::active = new Scene();
		pSceneObject cam = SceneObject::New(Vec3());
		Scene::AddObject(cam);
		mainCamera(cam->AddComponent<Camera>());

		glFrontFace(GL_CW);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glClearColor(0, 0, 0, 1.f);

		glfwSetWindowSize(window, scrW, scrH);
		ReshapeGL(window, scrW, scrH);

		glfwSetWindowSizeCallback(window, ReshapeGL);
		glfwSetCursorPosCallback(window, MotionGL);
		glfwSetMouseButtonCallback(window, MouseGL);
		glfwSetScrollCallback(window, MouseScrGL);
		glfwSetCursorEnterCallback(window, MouseEnterGL);
		glfwSetDropCallback(window, DropGL);
		glfwSetWindowFocusCallback(window, FocusGL);


		initd = 2;
		Time::millis = milliseconds();
	}
}

bool ChokoLait::alive() {
	return !glfwWindowShouldClose(window);
}

void ChokoLait::Update(emptyCallbackFunc func) {
	Input::PreLoop();
	Time::Update();
	Input::UpdateMouseNKeyboard();
	if (func) func();
}

void ChokoLait::Paint(emptyCallbackFunc rendFunc, emptyCallbackFunc paintFunc) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	mainCamera->Render(rendFunc);

	MVP::Switch(false);
	MVP::Clear();

	UI::PreLoop();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glDepthMask(true);

	if (paintFunc) paintFunc();

	glfwSwapBuffers(window);
	glfwPollEvents();
}

void ChokoLait::MouseGL(GLFWwindow* window, int button, int state, int mods) {
	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:
		Input::mouse0 = (state == GLFW_PRESS);
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		Input::mouse1 = (state == GLFW_PRESS);
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		Input::mouse2 = (state == GLFW_PRESS);
		break;
	}
}

void ChokoLait::MouseScrGL(GLFWwindow* window, double xoff, double yoff) {
	Input::_mouseScroll = (float)yoff;
}

void ChokoLait::MouseEnterGL(GLFWwindow* window, int e) {
	
}

void ChokoLait::MotionGL(GLFWwindow* window, double x, double y) {
	Input::mousePos = Vec2(x, y);
	Input::mousePosRelative = Vec2(x*1.f / Display::width, y*1.f / Display::height);
}

void ChokoLait::ReshapeGL(GLFWwindow* window, int w, int h) {
#ifdef SIMULATE_FRAMESCALE
	w /= SIMULATE_FRAMESCALE;
	h /= SIMULATE_FRAMESCALE;
#endif
	Display::width = (int)(w * Display::dpiScl);
	Display::height = (int)(h * Display::dpiScl);
	Display::actualWidth = w;
	Display::actualHeight = h;
	glfwGetFramebufferSize(window, &Display::frameWidth, &Display::frameHeight);
	glViewport(0, 0, Display::frameWidth, Display::frameHeight);
}

void ChokoLait::DropGL(GLFWwindow* window, int w, const char** c) {
	for (auto& a : dropFuncs) {
		if (a(w, c))
			break;
	}
}

void ChokoLait::FocusGL(GLFWwindow* window, int focus) {
	if (!!focus) {
		for (auto& a : focusFuncs) {
			a();
		}
	}
}