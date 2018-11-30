#include "node_info.h"
#include "node_inputs.h"
#include "web/anweb.h"
#include "md/particles.h"

Node_Info::Node_Info() :AnNode(new DmScript(sig), AN_FLAG_NOSAVECONV) {
	title = "System Info";
	titleCol = NODE_COL_IO;
	canTile = true;
	
	AddOutput();
	script->AddOutput("atom count", "int");
	AddOutput();
	script->AddOutput("frame count", "int");
	AddOutput();
	script->AddOutput("current frame", "int");
	AddOutput(CVar("bounding box", 'd', 1, { nullptr }, { 6 }));
	script->AddOutput(conV.back());
}

void Node_Info::Execute() {
	conV[0].value = &Particles::particleSz;
	conV[1].value = &Particles::anim.frameCount;
	conV[2].value = &Node_Inputs::frame;
	static double* bbx = nullptr;
	if (!Particles::anim.bboxs.size()) bbx = &Particles::boundingBox[0];
	else if (!AnWeb::execFrame) bbx = &Particles::anim.bboxs[Particles::anim.currentFrame][0];
	else bbx = &Particles::anim.bboxs[AnWeb::execFrame-1][0];
	conV[3].value = &bbx;
}