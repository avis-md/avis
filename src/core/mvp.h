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
#include "Engine.h"

class MVP {
public:
	static void Reset();
	static void Switch(bool isProj);
	static void Push(), Pop(), Clear();
	static void Mul(const Mat4x4& mat);
	static void Translate(const Vec3& v), Translate(float x, float y, float z);
	static void Scale(const Vec3& v), Scale(float x, float y, float z);

	static Mat4x4 modelview(), projection();

	static Mat4x4 top_p();
protected:
	class stack : public std::stack<Mat4x4> {
	public:
		using std::stack<Mat4x4>::c;
	};

	static stack MV, P;
	static Mat4x4 _mv, _p;
	static bool changedMv, changedP;
	static Mat4x4 identity;
	static bool isProj;

};