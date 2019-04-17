#pragma once
#include "Engine.h"
#include "OpenImageDenoise/oidn.hpp"

#ifdef PLATFORM_WIN
#pragma comment(lib, "OpenImageDenoise.lib")
#endif

class Denoise {
	static oidn::DeviceRef device;
	static oidn::FilterRef filter;

public:
	typedef oidn::Format FMT;

	static void Init();
	
	//Denoise an image. Image format must be RGB.
	static void Apply(float* img, int w, int h, float* res);
};