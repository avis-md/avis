#pragma once

/*
forked from ChokoEngine -- Chokomancarr 2018
*/

#include "../../appversion.h"
#define VERSIONSTRING "version " APPVERSION

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define STRINGMRGDO(a,b) a ## b
#define STRINGMRG(a,b) STRINGMRGDO(a,b)

#define push_val(var, nm, val) auto var = nm; nm = val;

#define OHNO(nm, msg) Debug::Error(#nm, "Something happened that should not! Please contact the monkey!\n  Information the monkey needs: " msg)

#pragma region includes
/* OS-specific headers */
#if defined(PLATFORM_WIN)
#include "minimalwin.h"
#include <WinSock2.h>
#pragma comment(lib, "Secur32.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#else
#include <arpa/inet.h>
#if defined(PLATFORM_LNX) || defined(PLATFORM_OSX)
#include <unistd.h>
#include <execinfo.h>
#endif
#endif

#include <signal.h>
#ifndef PLATFORM_WIN
#define __debugbreak() raise(SIGTRAP)
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>
typedef GLFWwindow NativeWindow;
#ifdef PLATFORM_WIN
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif

#include <complex>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stack>
#include <map>
#include <unordered_map>
#include <array>
#include <memory>
#include <thread>
#include <cmath>
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#pragma region Type Extensions

typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;

typedef glm::tvec2<int, glm::highp> Int2;
typedef glm::vec2 Vec2;
typedef glm::tvec2<int, glm::highp> Int3;
typedef glm::vec3 Vec3;
typedef glm::tvec3<int, glm::highp> Int4;
typedef glm::vec4 Vec4;
typedef glm::quat Quat;
typedef glm::mat4 Mat4x4;

const float PI = 3.1415926535f;
const float rad2deg = 57.2958f;
const float deg2rad = 0.0174533f;
const char char0 = 0;

const uint UI_MAX_EDIT_TEXT_FRAMES = 8;

float Dw(float), Dh(float);
Vec3 Ds(Vec3);
Vec2 Ds2(Vec2);

#include "core/math.h"

#ifdef PLATFORM_WIN
#define stat _stat
#else
void fopen_s(FILE** f, const char* c, const char* m);
#define sscanf_s sscanf
#define _putenv_s(nm, loc) setenv(nm, loc, 1)
#endif

#define F2ISTREAM(_varname, _pathname) std::ifstream _f2i_ifstream((_pathname).c_str(), std::ios::in | std::ios::binary); \
std::istream _varname(_f2i_ifstream.rdbuf());

#include "utils/net.h"
#include "utils/xml.h"
#include "utils/strext.h"
#include "utils/glmext.h"

#include "core/mvp.h"
#include "core/bbox.h"

#include "utils/color.h"
#include "utils/rect.h"

#pragma endregion

#pragma region class names

template <class T> class Ref;

#define _canref(obj) class obj; \
typedef std::shared_ptr<obj> p ## obj; \
typedef Ref<obj> r ## obj;

//SceneObjects.h
_canref(Component);
_canref(Transform);
_canref(SceneObject);
_canref(Camera);

//AssetObjects.h
class AssetItem;
class AssetManager;
_canref(AssetObject);

_canref(Background);
_canref(Mesh);
_canref(Shader);
_canref(Texture);
_canref(RenderTexture);

#pragma endregion

#pragma region enums

enum MOUSE_STATUS : byte {
	MOUSE_NONE = 0x00,
	MOUSE_DOWN,
	MOUSE_HOLD,
	MOUSE_UP,
	MOUSE_R_DOWN,
	MOUSE_R_HOLD,
	MOUSE_R_UP,
	MOUSE_HOVER_FLAG = 0x10,
	MOUSE_CLICK, //use for buttons
	MOUSE_PRESS, //use for buttons
	MOUSE_RELEASE,  //use for buttons
	MOUSE_R_CLICK, //use for buttons
	MOUSE_R_PRESS, //use for buttons
	MOUSE_R_RELEASE,  //use for buttons
};

enum ALIGNMENT : byte {
	ALIGN_BOTLEFT = 0x00,
	ALIGN_BOTCENTER = 0x01,
	ALIGN_BOTRIGHT = 0x02,
	ALIGN_MIDLEFT = 0x10,
	ALIGN_MIDCENTER = 0x11,
	ALIGN_MIDRIGHT = 0x12,
	ALIGN_TOPLEFT = 0x20,
	ALIGN_TOPCENTER = 0x21,
	ALIGN_TOPRIGHT = 0x22
};

enum ORIENTATION : byte {
	ORIENT_NONE,
	ORIENT_HORIZONTAL,
	ORIENT_VERTICAL
};

enum TransformSpace : byte {
	Space_Self = 0x00,
	Space_World
};

enum ASSETTYPE : byte {
	ASSETTYPE_UNDEF = 0x00,

	ASSETTYPE_SCENE = 0x90,
	ASSETTYPE_TEXTURE = 0x01,
	ASSETTYPE_HDRI = 0x02,
	ASSETTYPE_TEXCUBE = 0x03,
	ASSETTYPE_SHADER = 0x05,
	ASSETTYPE_MATERIAL = 0x10,
	ASSETTYPE_BLEND = 0x20,
	ASSETTYPE_MESH = 0x21,
	ASSETTYPE_ANIMCLIP = 0x30,
	ASSETTYPE_ANIMATION = 0x31,
	ASSETTYPE_CAMEFFECT = 0x40,
	ASSETTYPE_AUDIOCLIP = 0x50,
	ASSETTYPE_SCRIPT_H = 0x9e,
	ASSETTYPE_SCRIPT_CPP = 0x9f,
	//derived types
	ASSETTYPE_TEXTURE_REND = 0xa0
};

enum InputKey {
	Key_None = 0,
	Key_Space = 32,
	Key_Apostrophe = 39,
	Key_Comma = 44, Key_Minus, Key_Dot, Key_Slash,
	Key_0 = 48, Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9,
	Key_Semicolon = 59, Key_Equal = 61,
	Key_A = 65, Key_B, Key_C, Key_D, Key_E, Key_F, Key_G, Key_H, Key_I, Key_J, Key_K, Key_L, Key_M, Key_N, Key_O, Key_P, Key_Q, Key_R, Key_S, Key_T, Key_U, Key_V, Key_W, Key_X, Key_Y, Key_Z,
	Key_LeftBracket = 91, Key_BackSlash, Key_RightBracket, Key_Accent = 96,
	Key_Escape = 256, Key_Enter, Key_Tab, Key_Backspace, Key_Insert, Key_Delete,
	Key_RightArrow, Key_LeftArrow, Key_DownArrow, Key_UpArrow, Key_PageUp, Key_PageDown, Key_Home, Key_End, Key_CapsLock, Key_ScrollLock, Key_NumLock,
	Key_F1 = 290, Key_F2, Key_F3, Key_F4, Key_F5, Key_F6, Key_F7, Key_F8, Key_F9, Key_F10, Key_F11, Key_F12,
	Key_NumPad0 = 320, Key_NumPad1, Key_NumPad2, Key_NumPad3, Key_NumPad4, Key_NumPad5, Key_NumPad6, Key_NumPad7, Key_NumPad8, Key_NumPad9,
	Key_NumPadDot, Key_NumPadDivide, Key_NumPadMultiply, Key_NumPadMinus, Key_NumPadAdd, Key_NumPadEnter, Key_NumPadEqual,
	Key_LeftShift = 340, Key_LeftControl, Key_LeftAlt, Key_RightShift = 344, Key_RightControl, Key_RightAlt
};

enum OBJECT_STATUS : byte {
	OBJECT_UNDEF,
	OBJECT_ALIVE,
	OBJECT_DEAD
};

enum DRAWTEX_SCALING : byte {
	DRAWTEX_FIT,
	DRAWTEX_CROP,
	DRAWTEX_STRETCH
};

enum COMPONENT_TYPE : byte {
	COMP_UNDEF = 0x00,
	COMP_CAM = 0x01, //camera
	COMP_MFT = 0x02, //mesh filter
	COMP_MRD = 0x10, //mesh renderer
	COMP_TRD = 0x11, //texture renderer
	COMP_SRD = 0x12, //skinned mesh renderer
	COMP_VRD = 0x13, //voxel renderer
	COMP_LHT = 0x20, //light
	COMP_RFQ = 0x22, //reflective quad
	COMP_RDP = 0x25, //render probe
	COMP_ARM = 0x30, //armature
	COMP_ANM = 0x31, //animator
	COMP_INK = 0x35, //inverse kinematics
	COMP_AUS = 0x40, //audio source
	COMP_AUL = 0x41, //audio listener
	COMP_PST = 0x50, //particle system
	COMP_SCR = 0xff //script
};

enum FFT_WINDOW : byte {
	FFT_WINDOW_RECTANGLE,
	FFT_WINDOW_HAMMING,
	FFT_WINDOW_BLACKMAN
};

#pragma endregion

#include "scene/object.h"

#include "core/debug.h"
#include "core/time.h"
#include "core/io.h"

#include "utils/random.h"
#include "utils/ptrext.h"
#include "utils/stream.h"
#include "utils/shader.h"

#include "core/font.h"
#include "core/input.h"
#include "core/display.h"
#include "core/ui.h"
#include "core/ui3.h"

#include "utils/fft.h"

#include "AssetObjects.h"

class Engine { //why do I have this class again?
public:
	static void BeginStencil(float x, float y, float w, float h);
	static void PushStencil(float x, float y, float w, float h);
	static void PopStencil();
	static void EndStencil();
	static void DrawLine(Vec2 v1, Vec2 v2, Vec4 col, float width);
	static void DrawLine(Vec3 v1, Vec3 v2, Vec4 col, float width);
	static void DrawLineW(Vec3 v1, Vec3 v2, Vec4 col, float width);
	static void DrawLinesW(Vec3* pts, int num, Vec4 col, float width);
	static void DrawCubeLinesW(Vec3 pos, float dx, float dy, float dz, Vec4 col);
	static MOUSE_STATUS Button(float x, float y, float w, float h);
	static MOUSE_STATUS Button(float x, float y, float w, float h, Vec4 normalVec4);
	static MOUSE_STATUS Button(float x, float y, float w, float h, Vec4 normalVec4, std::string label, float labelSize, Vec4 labelCol, bool labelCenter = false, Font* labelFont = UI::font);
	static MOUSE_STATUS Button(float x, float y, float w, float h, Vec4 normalVec4, Vec4 highlightVec4, Vec4 pressVec4);
	static MOUSE_STATUS Button(float x, float y, float w, float h, const Texture& texture, Vec4 normalVec4 = white(0.8f), Vec4 highlightVec4 = white(), Vec4 pressVec4 = white(1, 0.5f), float uvx = 0, float uvy = 0, float uvw = 1, float uvh = 1);
	static MOUSE_STATUS Button(float x, float y, float w, float h, Vec4 normalVec4, Vec4 highlightVec4, Vec4 pressVec4, std::string label, float labelSize, Font* labelFont, Vec4 labelVec4, bool labelCenter = false);
	static bool Toggle(float x, float y, float s, Vec4 col, bool t);
	static bool Toggle(float x, float y, float s, const Texture& texture, bool t, Vec4 col=white(), ORIENTATION o = ORIENT_NONE);
	static float DrawSliderFill(float x, float y, float w, float h, float min, float max, float val, Vec4 background, Vec4 foreground);
	static float DrawSliderFillY(float x, float y, float w, float h, float min, float max, float val, Vec4 background, Vec4 foreground);
	static Vec2 DrawSliderFill2D(float x, float y, float w, float h, Vec2 min, Vec2 max, Vec2 val, Vec4 background, Vec4 foreground);
	//scaleType: 0=scale, 1=clip, 2=tile
	static void DrawProgressBar(float x, float y, float w, float h, float progress, Vec4 background, const Texture& foreground, Vec4 tint, int padding, byte scaleType);

	static void RotateUI(float a, Vec2 point);
	static void ResetUIMatrix();

	static void Sleep(uint ms);

	static GLuint defProgram, defProgramW, unlitProgram, unlitProgramA, unlitProgramC, skyProgram;
	static GLint defColLoc, defWColLoc, defWMVPLoc;
	
	PROGDEF_H(lineWProg, 5)

	static void InitShaders();

	static GLuint quadBuffer;
	static GLint drawQuadLocs[3], drawQuadLocsA[3], drawQuadLocsC[1];

	static void DrawCube(Vec3 pos, float dx, float dy, float dz, Vec4 col);

	static std::thread::id _mainThreadId;

	static void Init();

	static std::vector<Rect> stencilRects;
	static Rect* stencilRect;

	static std::mutex stateLock, stateLock2;
	static int stateLockId;
	static std::mutex stateLockCV_m;
	static std::condition_variable stateLockCV;
	static void AcquireLock(int i);
	static void ReleaseLock();
	static void WaitForLockValue();
};

#include "SceneObjects.h"