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

Component::Component(std::string name, COMPONENT_TYPE t, DRAWORDER drawOrder, SceneObject* o, std::vector<COMPONENT_TYPE> dep)
	: Object(name), componentType(t), drawOrder(drawOrder), active(true), dependancies(dep), _expanded(true) {
	//for (COMPONENT_TYPE t : dependancies) {
	//	dependacyPointers.push_back(rComponent());
	//}
	if (o) object(o);
}

COMPONENT_TYPE Component::Name2Type(std::string nm) {
	static const std::string names[] = {
		"Camera",
		"MeshFilter",
		"MeshRenderer",
		"TextureRenderer",
		"SkinnedMeshRenderer",
		"ParticleSystem",
		"Light",
		"ReflectiveQuad",
		"RenderProbe",
		"Armature",
		"Animator",
		"InverseKinematics",
		"Script"
	};
	static const COMPONENT_TYPE types[] = {
		COMP_CAM,
		COMP_MFT,
		COMP_MRD,
		COMP_TRD,
		COMP_SRD,
		COMP_PST,
		COMP_LHT,
		COMP_RFQ,
		COMP_RDP,
		COMP_ARM,
		COMP_ANM,
		COMP_INK,
		COMP_SCR
	};

	for (uint i = 0; i < sizeof(types); ++i) {
		if ((nm) == names[i]) return types[i];
	}
	return COMP_UNDEF;
}