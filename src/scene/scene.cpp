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

#include "Engine.h"

bool Scene::dirty = false;

Scene* Scene::active = nullptr;
std::ifstream* Scene::strm = nullptr;
std::vector<std::string> Scene::sceneNames = {};
std::vector<long> Scene::scenePoss = {};

Scene::Scene() : sceneName("newScene"), settings() {
	std::vector<pSceneObject>().swap(objects);
}

void Scene::AddObject(pSceneObject object, pSceneObject parent) {
	if (!active)
		return;
	if (!parent) {
		active->objects.push_back(object);
		active->objectCount++;
	}
	else
		parent->AddChild(object);
}

void Scene::DeleteObject(pSceneObject o) {
	if (!active)
		return;
	o->dead = true;
}

void Scene::Unload() {

}

void Scene::CleanDeadObjects() {
	for (int a = active->objectCount - 1; a >= 0; --a) {
		if (active->objects[a]->dead) {
			active->objects.erase(active->objects.begin() + a);
			active->objectCount--;
		}
	}
}