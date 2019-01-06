#include "node_setbboxcenter.h"
#include "md/particles.h"
#include "web/anweb.h"

INODE_DEF(__("Set Bounding Box Center"), SetBBoxCenter, SET)

Node_SetBBoxCenter::Node_SetBBoxCenter() : INODE_INIT {
	INODE_TITLE(NODE_COL_IO);
	
	AddInput();
	script->AddInput("center", "list(1d)");
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