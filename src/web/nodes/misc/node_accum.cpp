#include "node_accum.h"
#include "md/particles.h"
#include "web/anweb.h"
#include "ui/ui_ext.h"

INODE_DEF(__("Accumulate"), Accum, MISC)

Node_Accum::Node_Accum() : INODE_INIT {
	INODE_TITLE(NODE_COL_SPC)
	INODE_SINIT(
		scr->AddInput(_("value"), AN_VARTYPE::ANY);
		scr->AddOutput(_("result"), AN_VARTYPE::ANY, 1);
	);
	
	IAddConV(0, { (int*)&Particles::anim.frameCount }, {});
}

void Node_Accum::DrawHeader(float& off) {
	AnNode::DrawHeader(off);
	UI2::Toggle(pos.x + 5, off, width - 10, "Block", block); off += 17;
}

void Node_Accum::Execute() {
	auto& vr = inputR[0].getvar();
	if (vr.type == AN_VARTYPE::LIST) {
		throw "Accumulate does not support lists yet!";
		return;
	}
	std::memcpy(&vals[AnWeb::realExecFrame*vr.stride], *(void**)inputR[0].getval(ANVAR_ORDER::C), vr.stride);
}

void Node_Accum::OnConn(bool o, int i) {
	if (!o) {
		Disconnect(0, true);
		vals.resize(Particles::anim.frameCount * inputR[0].getvar().stride);
		auto& ov = ((DmScript_I*)script.get())->outputVs;
		ov[0].val.p = vals.data();
		auto& ot = script->parent->outputs[0];
		ot.itemType = inputR[0].getvar().type;
		ot.InitName();
	}
}