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

#include "annode_internal.h"
#include "web/anweb.h"

std::array<std::vector<AnNode_Internal::noderef>, (int)ANNODE_GROUP::_COUNT> AnNode_Internal::scrs;
std::array<std::string, (int)ANNODE_GROUP::_COUNT> AnNode_Internal::groupNms;

void AnNode_Internal::Init() {
	for (auto& ss : scrs) {
		for (auto& a : ss) {
			a.name = _(a.name.c_str());
		}
	}
	groupNms[0] = _("Readers");
	groupNms[1] = _("Writers");
	groupNms[2] = _("Modifiers");
	groupNms[3] = _("Converters");
	groupNms[4] = _("Miscellaneous");
}

AnNode_Internal_Reg::AnNode_Internal_Reg(const std::string sig, const std::string nm, ANNODE_GROUP grp, spawnFunc func) {
	AnNode_Internal::scrs[(int)grp].push_back(AnNode_Internal::noderef(sig, nm, func));
}