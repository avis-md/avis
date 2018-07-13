#include "Engine.h"

bool Scene::dirty = false;

Scene* Scene::active = nullptr;
std::ifstream* Scene::strm = nullptr;
#ifndef IS_EDITOR
std::vector<string> Scene::sceneEPaths = {};
#endif
std::vector<string> Scene::sceneNames = {};
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
	for (int a = active->objectCount - 1; a >= 0; a--) {
		if (active->objects[a]->dead) {
			active->objects.erase(active->objects.begin() + a);
			active->objectCount--;
		}
	}
}