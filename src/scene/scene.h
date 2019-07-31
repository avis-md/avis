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

class SceneSettings {
public:
	Background* sky;
	float skyStrength, skyBrightness;
	Color ambientCol;

	bool useFog, sunFog;
	float fogDensity, fogSunSpread;
	Vec4 fogColor, fogSunColor;
	
	friend class Scene;
protected:
	SceneSettings() : sky(nullptr), skyStrength(1), skyBrightness(1), skyId(-1) {
		sky = 0;
	}

	SceneSettings(const SceneSettings&);
	SceneSettings& operator= (const SceneSettings&);

	int skyId;
};

/*! A scene contains everything in a level.
*/
class Scene {
public:
	Scene();

	std::string sceneName;

	/*! The current active scene.
	In Lait versions, this value cannot be changed.
	*/
	static Scene* active;

	uint objectCount = 0;
	std::vector<pSceneObject> objects;
	SceneSettings settings;
	std::vector<Component*> _preUpdateComps;
	std::vector<Component*> _preLUpdateComps;
	std::vector<Component*> _preRenderComps;

	static bool dirty;

	static void AddObject(pSceneObject object, pSceneObject parent = nullptr);
	static void DeleteObject(pSceneObject object);

	friend int main(int argc, char **argv);
	friend class AssetManager;
	friend class Component;

protected:
	static std::ifstream* strm;
	static std::vector<std::string> sceneNames;
	static std::vector<long> scenePoss;

	static void ReadD0();
	static void Unload();
	static void CleanDeadObjects();
};
