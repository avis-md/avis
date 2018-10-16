#include "avi.h"

AVI::AVI(const std::string& path, uint w, uint h, uint fps) {
	gwavi = gwavi_open(path.c_str(), w, h, "MJPG", fps);
}

void AVI::AddFrame(GLuint tex) {
	

	gwavi_add_frame(gwavi, buf, bufsz);
}

void AVI::End() {
	gwavi_close(gwavi);
}
