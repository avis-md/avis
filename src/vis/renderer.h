// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

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

	static enum class STATUS {
		READY,
		BUSY,
		IMG,
	} status;

	static bool imgUseAlpha;
	static uint imgW, imgH, vidW, vidH;
	static uint imgSlices;
	static bool imgShowAxes;
	static float imgAxesScale;
	static uint imgMsaa, vidMsaa;
	static uint vidMaxFrames;
	
	static float resLerp;

	static std::string outputFolder;

	static GLuint res_fbo, res_img, res_dph;
	static GLuint tmp_fbo, tmp_img, tmp_dph;

	static int _maxTexSz;

	static void Init();

	static void Draw(), DrawMenu();

	static void ToImage();
	static void ToVid();

	static void MakeTex(GLuint& fbo, GLuint& tex, int w, int h);
};