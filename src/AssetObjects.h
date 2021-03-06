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

#pragma region enums

enum Interpolation : byte {
	Interpolation_Ease,
	Interpolation_Linear,
	Interpolation_Before,
	Interpolation_Center,
	Interpolation_After
};

enum AnimKeyType : byte {
	AK_Undef = 0x00,
	AK_BoneLoc = 0x10,
	AK_BoneRot,
	AK_BoneScl,
	AK_Location = 0x20,
	AK_Rotation,
	AK_Scale,
	AK_ShapeKey = 0x30,
};

enum TEX_FILTERING : byte {
	TEX_FILTER_POINT = 0x00,
	TEX_FILTER_BILINEAR,
	TEX_FILTER_TRILINEAR
};

enum TEX_WRAPING : byte {
	TEX_WRAP_CLAMP,
	TEX_WRAP_REPEAT
};

enum TEX_TYPE : byte {
	TEX_TYPE_NORMAL = 0x00,
	TEX_TYPE_RENDERTEXTURE,
	TEX_TYPE_READWRITE,
	TEX_TYPE_UNDEF
};

enum RT_FLAGS : byte {
	RT_FLAG_NONE = 0U,
	RT_FLAG_DEPTH = 1U,
	RT_FLAG_STENCIL = 2U, //doesn't do anything for now
	RT_FLAG_HDR = 4U
};

enum CAM_CLEARTYPE : byte {
	CAM_CLEAR_NONE,
	CAM_CLEAR_COLOR,
	CAM_CLEAR_DEPTH,
	CAM_CLEAR_SKY
};

#pragma endregion

#include "asset/mesh.h"
#include "asset/texture.h"
#include "asset/texture3d.h"