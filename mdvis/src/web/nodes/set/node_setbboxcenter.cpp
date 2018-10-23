#include "node_setbboxcenter.h"
#include "md/particles.h"

Node_SetBBoxCenter::Node_SetBBoxCenter() : AnNode(new DmScript(sig)) {
    title = "Set Bounding Box Center";
    titleCol = NODE_COL_IO;
    inputR.resize(1);
    auto& vrs = script->invars;
    script->invaropts.resize(1);
	script->invars.resize(1, std::pair<std::string, std::string>("center", "list(1d)"));
}

void Node_SetBBoxCenter::Execute() {
	if (!inputR[0].first) return;
	Particles::Rebound(**(glm::dvec3**)inputR[0].getval());
}