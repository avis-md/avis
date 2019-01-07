#include "node_accum.h"
#include "md/particles.h"
#include "web/anweb.h"
#include "ui/ui_ext.h"

INODE_DEF(__("Accumulate"), Accum, MISC)

Node_Accum::Node_Accum() : INODE_INIT {
	INODE_TITLE(NODE_COL_MOD)
	AddInput();
	script->AddInput("value", "*");
	AddOutput(CVar("result", 'd', 0, {}, {}));
	script->AddOutput("accumulated", "list(??)");
}

void Node_Accum::DrawHeader(float& off) {
	AnNode::DrawHeader(off);
	UI::Quad(pos.x, off, width, 17, bgCol);
	UI2::Toggle(pos.x + 5, off, width - 10, "Block", block); off += 17;
}

void Node_Accum::Execute() {
	auto& cv = inputR[0].getconv();
	if (cv.type == AN_VARTYPE::LIST) {
		throw "Accumulate does not support lists yet!";
		return;
	}
	std::memcpy(&vals[AnWeb::realExecFrame*cv.stride], *(void**)cv.value, cv.stride);
}

void Node_Accum::OnConn(bool o, int i) {
	if (!o) {
		vals.resize(Particles::anim.frameCount * inputR[0].getconv().stride);
		conV[0].value = vals.data();
		Disconnect(0, true);
	}
}