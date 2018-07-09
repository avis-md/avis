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

void Scene::ReadD0() {
#ifndef IS_EDITOR
	ushort num;
	_Strm2Val(*strm, num);
	for (ushort a = 0; a < num; a++) {
		uint sz;
		char c[100];
		if (_pipemode) {
			strm->getline(c, 100);
			sceneNames.push_back(c);
			sceneEPaths.push_back(AssetManager::eBasePath + sceneNames[a]);
			Debug::Message("AssetManager", "Registered scene " + sceneNames[a]);
		}
		else {
			_Strm2Val(*strm, sz);
			*strm >> c[0];
			long p1 = (long)strm->tellg();
			scenePoss.push_back(p1);
			*strm >> c[0] >> c[1];
			if (c[0] != 'S' || c[1] != 'N') {
				Debug::Error("Scene Importer", "fatal: Scene Signature wrong!");
				return;
			}
			strm->getline(c, 100, 0);
			sceneNames.push_back(c);
			strm->seekg(p1 + sz + 1);
			Debug::Message("AssetManager", "Registered scene " + sceneNames[a] + " (" + std::to_string(sz) + " bytes)");
		}
	}
#endif
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