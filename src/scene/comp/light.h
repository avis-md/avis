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
#include "SceneObjects.h"

struct RSM_RANDOM_BUFFER {
	float xPos[1024];
	float yPos[1024];
	float size[1024];
};

enum LIGHTTYPE : byte {
	LIGHTTYPE_POINT,
	LIGHTTYPE_DIRECTIONAL,
	LIGHTTYPE_SPOT,
};

enum LIGHT_FALLOFF : byte {
	LIGHT_FALLOFF_INVERSE,
	LIGHT_FALLOFF_INVSQUARE,
	LIGHT_FALLOFF_LINEAR,
	LIGHT_FALLOFF_CONSTANT
};
#define LIGHT_POINT_MINSTR 0.01f
#define BUFFERLOC_LIGHT_RSM 2
class Light : public Component {
public:
	Light();
	LIGHTTYPE lightType;

	float intensity = 1;
	Vec4 color = white();
	float angle = 60;
	float minDist = 0.01f, maxDist = 5;
	bool drawShadow = true, shadowOnly = false;
	float shadowBias = 0.001f, shadowStrength = 1;
	bool contactShadows = false;
	float contactShadowDistance = 0.1f;
	uint contactShadowSamples = 20;
	Texture* cookie = 0;
	float cookieStrength = 1;
	bool square = false;
	LIGHT_FALLOFF falloff = LIGHT_FALLOFF_INVSQUARE;
	Texture* hsvMap = 0;

	void DrawShadowMap(GLuint tar = 0);

	friend class Camera;
	friend class Engine;
protected:
	Light(std::ifstream& stream, SceneObject* o, long pos = -1);

	static void InitShadow();
	void CalcShadowMatrix();
	static GLuint _shadowFbo, _shadowGITexs[3], _shadowMap;
	static GLuint _shadowCubeFbos[6], _shadowCubeMap;
	static GLuint _fluxFbo, _fluxTex, _rsmFbo, _rsmTex, _rsmUBO;
	static RSM_RANDOM_BUFFER _rsmBuffer;

	static std::vector<GLint> paramLocs_Point, paramLocs_Spot, paramLocs_SpotCS, paramLocs_SpotFluxer, paramLocs_SpotRSM; //make writing faster
};
