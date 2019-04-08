#include "ChokoLait.h"
#include "vis/system.h"

#ifdef PLATFORM_OSX
#include </usr/include/mach-o/getsect.h>
#include </usr/include/mach-o/dyld.h>

std::string _mac_debug_base_address = "";
#endif

#define USE_DEBUG_CONTEXT
//#define SIMULATE_FRAMESCALE 2

void glDebugOutput(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	const void *userParam);

int ChokoLait::initd = 0;
pSceneObject ChokoLait::mainCameraObj;
pCamera ChokoLait::mainCamera;
GLFWwindow* ChokoLait::window = nullptr;
bool ChokoLait::foreground;
std::vector<dropFileFunc> ChokoLait::dropFuncs;
std::vector<emptyCallbackFunc> ChokoLait::focusFuncs;

#define _CHOKOLAIT_MAGIC 0x30cd84
uint CHOKOLAIT_MAGIC = _CHOKOLAIT_MAGIC;

void _printstack() {
	Debug::Error("System", "Dumping stack trace...");
	void* buf[50];
	uint c = Debug::StackTrace(50, buf);
#ifndef PLATFORM_WIN
	remove((IO::path + "trace.txt").c_str());
	auto bs = backtrace_symbols(buf, c);
	for (uint a = 0; a < c; ++a) {
		std::string str(bs[a]);
		if (!Debug::suppress) Debug::Error("System", str);
		int pos = string_find(str, "avis");
		if (pos > -1) {
			str = str.substr(string_find(str, "(+") + 2);
			str = str.substr(0, str.find_first_of(")"));
#ifdef PLATFORM_LNX
			auto cmd = ("addr2line -a " + str + " -e " + IO::path + "avis >> " + IO::path + "trace.txt");
#else
			auto cmd = ("atos -o " + IO::path + "avis " + "-l " + _mac_debug_base_address + " " + str + " >> " + IO::path + "trace.txt");
#endif
			system(cmd.c_str());
		}
	}
	if (!Debug::suppress) std::cout << "monkey-readable text below:" << std::endl;
	else std::cout << "monkey-readable text below:" << std::endl;
#ifdef PLATFORM_LNX
	system(("cat " + IO::path + "trace.txt").c_str());
#else
	system(("cat " + IO::path + "trace.txt | grep \"(in avis)\"").c_str());
#endif
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
#endif
	signal(SIGABRT, &_dieded);
	signal(SIGFPE, &_sigfpe);
	signal(SIGSEGV, &_sigseg);
	signal(SIGTERM, &_sigtrm);

	IO::InitPath();
	VisSystem::InitEnv();
	Debug::Init();
	Debug::Message("IO", "Path is " + IO::path);

#ifdef PLATFORM_OSX
	const struct segment_command_64* command = getsegbyname("__TEXT");
    uint64_t addr0 = command->vmaddr;
	uint64_t addr1 = _dyld_get_image_vmaddr_slide(0);
	std::ostringstream ss;
	ss << std::hex << addr0 + addr1;
	_mac_debug_base_address = ss.str();
#endif

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

#ifndef PLATFORM_OSX
	glewExperimental = true;
	GLint GlewInitResult = glewInit();
	if (GLEW_OK != GlewInitResult)
	{
		Debug::Error("System", "Glew error: " + std::string((const char*)glewGetErrorString(GlewInitResult)));
		abort();
	}
#endif

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

		//Scene::active = new Scene();
		//Scene::AddObject(cam);
		mainCameraObj = SceneObject::New(Vec3());
		mainCamera = mainCameraObj->AddComponent<Camera>();

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
	
	auto w = Display::width;
	auto h = Display::height;
	Display::width *= Display::dpiScl;
	Display::height *= Display::dpiScl;
	mainCamera->Render(rendFunc);
	Display::width = w;
	Display::height = h;

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
		Input::_mouse0 = (state == GLFW_PRESS);
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		Input::_mouse1 = (state == GLFW_PRESS);
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		Input::_mouse2 = (state == GLFW_PRESS);
		break;
	}
}

void ChokoLait::MouseScrGL(GLFWwindow* window, double xoff, double yoff) {
	Input::_mouseScroll = (float)yoff * Input::scrollScl;
}

void ChokoLait::MouseEnterGL(GLFWwindow* window, int e) {
	
}

void ChokoLait::MotionGL(GLFWwindow* window, double x, double y) {
	Input::mousePos = Vec2(x, y) / Display::dpiScl;
	Input::mousePosRelative = Input::mousePos / Vec2(Display::width, Display::height);
}

void ChokoLait::ReshapeGL(GLFWwindow* window, int w, int h) {
#ifdef SIMULATE_FRAMESCALE
	w /= SIMULATE_FRAMESCALE;
	h /= SIMULATE_FRAMESCALE;
#endif
	Display::width = (int)(w / Display::dpiScl);
	Display::height = (int)(h / Display::dpiScl);
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
	foreground = !!focus;
	if (foreground) {
		for (auto& a : focusFuncs) {
			a();
		}
	}
}


#ifdef USE_DEBUG_CONTEXT
void glDebugOutput(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	const void *userParam)
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
