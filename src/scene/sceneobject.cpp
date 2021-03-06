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

SceneObject::SceneObject(Vec3 pos, Quat rot, Vec3 scale) : SceneObject("New Object", pos, rot, scale) {}
SceneObject::SceneObject(std::string s, Vec3 pos, Quat rot, Vec3 scale) : Object(s), _expanded(true) {}
SceneObject::SceneObject(byte* data) {}

SceneObject::~SceneObject() {

}

void SceneObject::SetActive(bool a, bool enableAll) {
	active = a;
}

bool SO_DoFromId(const std::vector<pSceneObject>& objs, uint id, pSceneObject& res) {
	for (auto a : objs) {
		if (a->id == id) {
			res = a;
			return true;
		}
		if (SO_DoFromId(a->children, id, res)) return true;
	}
	return false;
}

pSceneObject SceneObject::_FromId(const std::vector<pSceneObject>& objs, ulong id) {
	if (!id) return nullptr;
	pSceneObject res = 0;
	SO_DoFromId(objs, id, res);
	return res;
}


pComponent ComponentFromType(COMPONENT_TYPE t) {
	switch (t) {
	case COMP_CAM:
		return std::make_shared<Camera>();
	default:
		return nullptr;
	}
}

void SceneObject::SetParent(pSceneObject nparent, bool retainLocal) {
	if (parent()) parent->children.erase(std::find_if(parent->children.begin(), parent->children.end(), [this](pSceneObject o)-> bool {return o.get() == this; }));
	parent->childCount--;
	if (nparent) parent->AddChild(pSceneObject(this), retainLocal);
}

pSceneObject SceneObject::AddChild(pSceneObject child, bool retainLocal) {
	Vec3 t;
	Quat r;
	if (!retainLocal) {
		t = child->transform._position;
		r = child->transform._rotation;
	}
	children.push_back(child);
	childCount++;
	child->parent(this);
	child->transform._UpdateWMatrix(transform._worldMatrix);
	if (!retainLocal) {
		child->transform.position(t);
		child->transform.rotation(r);
	}
	return get_shared<SceneObject>(this);
}

pComponent SceneObject::AddComponent(pComponent c) {
	c->object(this);
	int i = 0;
	for (COMPONENT_TYPE t : c->dependancies) {
		c->dependacyPointers[i] = GetComponent(t);
		if (!c->dependacyPointers[i]) {
			c->dependacyPointers[i](AddComponent(ComponentFromType(t)));
		}
		i++;
	}
	for (pComponent cc : _components)
	{
		if ((cc->componentType == c->componentType) && cc->componentType != COMP_SCR) {
			Debug::Warning("Add Component", "Same component already exists!");
			return cc;
		}
	}
	_components.push_back(c);
	_componentCount++;
	Refresh();
	return c;
}

pComponent SceneObject::GetComponent(COMPONENT_TYPE type) {
	for (pComponent cc : _components)
	{
		if (cc->componentType == type) {
			return cc;
		}
	}
	return nullptr;
}

void SceneObject::RemoveComponent(pComponent c) {
	for (int a = _components.size() - 1; a >= 0; --a) {
		if (_components[a] == c) {
			for (int aa = _components.size() - 1; aa >= 0; --aa) {
				for (COMPONENT_TYPE t : _components[aa]->dependancies) {
					if (t == c->componentType) {
						Debug::Warning("SceneObject", "Cannot delete " + c->name + " because other components depend on it!");
						return;
					}
				}
			}
			_components.erase(_components.begin() + a);
			return;
		}
	}
	Debug::Warning("SceneObject", "component to delete is not found");
}

void SceneObject::Refresh() {
	for (pComponent c : _components) {
		c->Refresh();
	}
}