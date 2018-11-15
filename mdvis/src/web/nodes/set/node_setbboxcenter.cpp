#include "node_setbboxcenter.h"
#include "md/particles.h"
#include "web/anweb.h"

Node_SetBBoxCenter::Node_SetBBoxCenter() : AnNode(new DmScript(sig)) {
	title = "Set Bounding Box Center";
	titleCol = NODE_COL_IO;
	inputR.resize(1);
	script->invaropts.resize(1);
	script->invars.resize(1, std::pair<std::string, std::string>("center", "list(1d)"));
}

void Node_SetBBoxCenter::Execute() {
	if (!inputR[0].first) return;
	if (*inputR[0].getconv().dimVals[0] != 3) {
		std::cerr << "vector must be of length 3!" << std::endl;
		return;
	}
	if (!AnWeb::execFrame)
		Particles::Rebound(**(glm::dvec3**)inputR[0].getval());
	else
		Particles::ReboundF(**(glm::dvec3**)inputR[0].getval(), AnWeb::execFrame-1);
}