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

typedef byte DRAWORDER;

class Component : public Object {
public:
	Component(std::string name, COMPONENT_TYPE t, DRAWORDER drawOrder = 0x00, SceneObject* o = nullptr, std::vector<COMPONENT_TYPE> dep = {});
	virtual  ~Component() {}

	const COMPONENT_TYPE componentType = COMP_UNDEF;
	const DRAWORDER drawOrder;
	bool active;
	rSceneObject object;

	virtual void OnPreUpdate() {}
	virtual void OnPreLUpdate() {}
	virtual void OnPreRender() {}

	friend int main(int argc, char **argv);
	friend class Scene;
	friend class SceneObject;
protected:
	std::vector<COMPONENT_TYPE> dependancies;
	std::vector<rComponent> dependacyPointers;

	//bool serializable;
	//std::vector<pair<void*, void*>> serializedValues;

	bool _expanded;

	static COMPONENT_TYPE Name2Type(std::string nm);

	virtual void LoadDefaultValues() {} //also loads assets

	virtual void Refresh() {}
};
