#pragma once
#include "Engine.h"
extern "C" {
#include "libgwavi.h"
}

class AVI {
public:
	AVI(const std::string& path, uint w, uint h, uint fps = 30);
	void AddFrame(GLuint tex);
	void End();

private:
	struct gwavi_t* gwavi;
};
