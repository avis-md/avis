#pragma once
#include "Engine.h"
extern "C" {
#include "gwavi.h"
}

class AVI {
public:
	AVI(const std::string& path, uint w, uint h, uint fps = 30);
	void AddFrame(GLuint tex);
	void End();

private:
	struct gwavi_t* gwavi;
	uint _w, _h;
};
