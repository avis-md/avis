#include "annode_internal.h"
#include "web/anweb.h"

std::array<std::vector<AnNode_Internal::noderef>, ANNODE_GROUP_COUNT> AnNode_Internal::scrs;
std::array<std::string, ANNODE_GROUP_COUNT> AnNode_Internal::groupNms;

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

AnNode_Internal_Reg::AnNode_Internal_Reg(const std::string sig, const std::string nm, ANNODE_GROUP grp, AnNode* (*func)()) {
	AnNode_Internal::scrs[(int)grp].push_back(AnNode_Internal::noderef(sig, nm, func));
}