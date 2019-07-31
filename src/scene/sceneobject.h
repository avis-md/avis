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

class SceneObject : public Object {
public:
	virtual ~SceneObject();
	static pSceneObject New(Vec3 pos, Quat rot = Quat(1, 0, 0, 0), Vec3 scale = Vec3(1, 1, 1)) {
		auto p = std::make_shared<SceneObject>(pos, rot, scale);
		p->transform.Init(p, pos, rot, scale);
		return p;
	}
	static pSceneObject New(std::string s = "New Object", Vec3 pos = Vec3(), Quat rot = Quat(1, 0, 0, 0), Vec3 scale = Vec3(1, 1, 1)) {
		auto p = std::make_shared<SceneObject>(s, pos, rot, scale);
		p->transform.Init(p, pos, rot, scale);
		return p;
	}

	bool active = true;
	Transform transform;

	void SetActive(bool active, bool enableAll = false);

	rSceneObject parent;
	uint childCount = 0;
	std::vector<pSceneObject> children;

	void SetParent(pSceneObject parent, bool retainLocal = false);
	/*! Add child to this SceneObject.
	*  @param child The child SceneObject to add.
	*  @param retainLocal If true, the child's local transform is retained. If false, the child's world transform is retained.
	*/
	pSceneObject AddChild(pSceneObject child, bool retainLocal = false);
	pSceneObject GetChild(int i) { return children[i]; }
	pComponent AddComponent(pComponent c);
	template <typename T, class ...Args> std::shared_ptr<T> AddComponent(Args&& ...args) {
		auto c = std::make_shared<T>(std::forward<Args>(args)...);
		AddComponent(std::static_pointer_cast<Component>(c));
		return c;
	}

	/*! you should probably use GetComponent<T>() instead.
	*/
	pComponent GetComponent(COMPONENT_TYPE type);
	template<class T> std::shared_ptr<T> GetComponent() {
		static_assert(std::is_base_of<Component, T>::value, "T is not a Component type!");
		for (pComponent cc : _components)
		{
			auto xx = std::dynamic_pointer_cast<T>(cc);
			if (xx != nullptr) {
				return xx;
			}
		}
		return nullptr;
	}
	void RemoveComponent(pComponent c);

	bool _expanded;
	int _componentCount;
	std::vector<pComponent> _components;

	SceneObject(Vec3 pos, Quat rot = Quat(1, 0, 0, 0), Vec3 scale = Vec3(1, 1, 1));
	SceneObject(std::string s = "New Object", Vec3 pos = Vec3(), Quat rot = Quat(1, 0, 0, 0), Vec3 scale = Vec3(1, 1, 1));
	SceneObject(byte* data);

	static pSceneObject _FromId(const std::vector<pSceneObject>& objs, ulong id);

	void Refresh();
};