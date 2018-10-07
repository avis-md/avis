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
	static uint imgSlices;
	
	static float resLerp;

	static std::string outputFolder;

	static GLuint res_fbo, res_img, res_dph;
	static GLuint tmp_fbo, tmp_img, tmp_dph;

	static int _maxTexSz;

	static void Init();

	static void Draw();

	static void ToImage();

	static void MakeTex(GLuint& fbo, GLuint& tex, int w, int h);
};