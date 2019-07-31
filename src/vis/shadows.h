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

class Shadows {
public:
	static pCamera cam;

	static bool show;
	static byte quality;
	static Vec3 pos, cpos;
	static float str, bias;
	static float rw, rz;
	static Quat rot;
	static float dst, dst2;
	static float box[6];
	static uint _sz;
	static Mat4x4 _p, _ip;

	static bool isPass;

	static GLuint _fbo, _dtex;

	static Shader _prog;;

	static void Init();
	static void UpdateBox();
	static void Rerender(), Reblit();
	static float DrawMenu(float off);

	static void Serialize(XmlNode* nd);

	static void Deserialize(XmlNode* nd);
};