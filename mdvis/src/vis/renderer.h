#pragma once
#include "ChokoLait.h"

class VisRenderer {
public:
	static enum class IMG_TYPE {
		PNG,
		JPG
	} imgType;
	static enum class VID_TYPE {
		AVI,
		GIF,
		PNG_SEQ
	} vidType;

	static enum STATUS {
		READY,
		BUSY,
		IMG,
	} status;

	static bool imgUseAlpha, vidUseAlpha;
	static uint imgW, imgH, vidW, vidH;
	
	static float resLerp;

	static std::string outputFolder;

	static GLuint res_fbo, res_img;

	static void Draw();

	static void ToImage();

	static void _SetRes();
};