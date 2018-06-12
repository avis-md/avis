#pragma once

#include "../../Defines.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <execinfo.h>
#include <signal.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <unordered_map>
#include <array>
#include <memory>
#include <thread>
#include <math.h>
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef std::string string;

typedef void Font;
typedef void Texture;
typedef int GLint;
typedef uint GLuint;

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

#define __debugbreak() raise(SIGTRAP)
void fopen_s(FILE** f, const char* c, const char* m);
#define sscanf_s sscanf
void _putenv_s(string nm, const char* loc);

Vec4 black(float f = 1);
Vec4 red(float f = 1, float i = 1), green(float f = 1, float i = 1), blue(float f = 1, float i = 1), cyan(float f = 1, float i = 1), yellow(float f = 1, float i = 1), white(float f = 1, float i = 1);

#include "io.h"
#include "debug.h"
#include "runcmd.h"
#include "dylib.h"
#include "strext.h"
#include "stream.h"
#include "math2.h"