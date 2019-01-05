#include "annode_internal.h"
#include "web/anweb.h"

const std::string Node_Inputs::sig = ".in";
const std::string Node_Info::sig = ".info";
const std::string Node_Vector::sig = ".vec";
const std::string Node_AddBond::sig = ".abnd";
const std::string Node_Camera_Out::sig = ".camo";
const std::string Node_Plot::sig = ".plot";
const std::string Node_GetAttribute::sig = ".gattr";
const std::string Node_SetAttribute::sig = ".sattr";
const std::string Node_ShowRange::sig = ".srng";
const std::string Node_SetRadScl::sig = ".rscl";
const std::string Node_SetBBoxCenter::sig = ".sbxc";
const std::string Node_AddSurface::sig = ".surf";
const std::string Node_TraceTrj::sig = ".trrj";
const std::string Node_Volume::sig = ".vol";
const std::string Node_Remap::sig = ".remap";
const std::string Node_Gromacs::sig = ".gro";
const std::string Node_AdjList::sig = ".adjl";
const std::string Node_AdjListI::sig = ".adjli";
const std::string Node_DoWhile::sig = ".dwhl";

std::array<std::vector<AnNode_Internal::noderef>, 4> AnNode_Internal::scrs;
std::array<std::string, 4> AnNode_Internal::groupNms;

void AnNode_Internal::Init() {
	for (auto& ss : scrs) {
		for (auto& a : ss) {
			a.name = _(a.name.c_str());
		}
	}
	groupNms[0] = _("Inputs");
	groupNms[1] = _("Modifiers");
	groupNms[2] = _("Generators");
	groupNms[3] = _("Miscellaneous");
}

AnNode_Internal_Reg::AnNode_Internal_Reg(const std::string sig, const std::string nm, ANNODE_GROUP grp, AnNode* (*func)()) {
	AnNode_Internal::scrs[(int)grp - 1].push_back(AnNode_Internal::noderef(sig, nm, func));
}