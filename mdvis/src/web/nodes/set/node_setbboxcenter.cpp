#include "node_setbboxcenter.h"
#include "md/particles.h"
#include "web/anweb.h"

INODE_DEF(__("Set Bounding Box Center"), SetBBoxCenter, SET)

Node_SetBBoxCenter::Node_SetBBoxCenter() : INODE_INIT{
	INODE_TITLE(NODE_COL_IO)
	INODE_SINIT(
		scr->AddInput(_("center"), AN_VARTYPE::DOUBLE, 1);
	);
}

void Node_SetBBoxCenter::Execute() {
	auto& ir = inputR[0];
	if (!ir.first) return;
	if (ir.getdim(0) != 3) {
		std::cerr << "vector must be of length 3!" << std::endl;
		return;
	}
	if (!AnWeb::execFrame)
		Particles::Rebound(**(glm::dvec3**)ir.getval());
	else
		Particles::ReboundF(**(glm::dvec3**)ir.getval(), AnWeb::execFrame - 1);
}