#pragma once

#define __VERSION__ "version 0.01"

#if defined(__APPLE__)
#ifndef PLATFORM_OSX
#define PLATFORM_OSX
#endif
#elif defined (__linux__)
#ifndef PLATFORM_LNX
#define PLATFORM_LNX
#endif
#elif defined (__WIN32__) || defined(_WIN32)
#ifndef PLATFORM_WIN
#define PLATFORM_WIN
#endif
#endif