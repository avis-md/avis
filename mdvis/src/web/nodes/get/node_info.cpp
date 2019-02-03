#include "node_info.h"
#include "web/anweb.h"
#include "md/particles.h"

INODE_DEF(__("System Info"), Info, GET)

Node_Info::Node_Info() : INODE_INITF(AN_FLAG_NOSAVECONV) {
	INODE_TITLE(NODE_COL_IO)
	/*
	const CVar cv("", AN_VARTYPE::INT);
	AddOutput(cv);
	scr.AddOutput(_("atom count"), "int");
	AddOutput(cv);
	scr.AddOutput(_("frame count"), "int");
	AddOutput(cv);
	scr.AddOutput(_("current frame"), "int");
	AddOutput(CVar("bounding box", 'd', 1, { nullptr }, { 6 }));
	scr.AddOutput(conV.back());
	conV[0].value = &Particles::particleSz;
	conV[1].value = &Particles::anim.frameCount;
	conV[2].value = &AnWeb::realExecFrame;
	*/
}

void Node_Info::Execute() {
	static double* bbx = nullptr;
	if (!Particles::anim.bboxs.size()) bbx = &Particles::boundingBox[0];
	else if (!AnWeb::execFrame) bbx = &Particles::anim.bboxs[Particles::anim.currentFrame][0];
	else bbx = &Particles::anim.bboxs[AnWeb::execFrame-1][0];
	//conV[3].value = &bbx;
}