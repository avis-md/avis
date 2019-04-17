#include "denoise.h"

oidn::DeviceRef Denoise::device;
oidn::FilterRef Denoise::filter;

void Denoise::Init() {
	Debug::Message("Denoise", "Initializing denoiser...");
	device = oidn::newDevice();
	device.commit();
	filter = device.newFilter("RT");
	filter.set("hdr", true);
}

void Denoise::Apply(float* img, int w, int h, float* res) {
	if (!filter) Init();

	filter.setImage("color", img, oidn::Format::Float3, w, h);
	filter.setImage("output", res, oidn::Format::Float3, w, h);
	filter.commit();
	filter.execute();
}