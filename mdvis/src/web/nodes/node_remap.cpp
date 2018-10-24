#include "node_remap.h"
#include "md/particles.h"
#include "ui/ui_ext.h"

Node_Remap::Node_Remap() : AnNode(new DmScript(sig)) {
	title = "Remap";
	titleCol = NODE_COL_NRM;
    canTile = false;
	inputR.resize(1);
	script->invars.push_back(std::pair<std::string, std::string>("values", "list(1f)"));

	outputR.resize(1);
	script->outvars.resize(1, std::pair<std::string, std::string>("result", "list(1f)"));
	
	conV.resize(1);
	auto& cv = conV[0];
	cv.type = AN_VARTYPE::LIST;
	cv.dimVals.resize(1);
	cv.value = &val0;
}

void Node_Remap::Execute() {
    if (!inputR[0].first) return;
	CVar& cv = inputR[0].first->conV[inputR[0].second];
	CVar& cvo = conV[0];
	auto sz = cvo.dimVals[0] = cv.dimVals[0];
	vals.resize(*sz);
	val0 = &vals[0];
	float* ins = *((float**)cv.value);
	for (int a = 0; a < *sz; ++a)  {
		vals[a] = graph.Eval(ins[a]);
	}
}

void Node_Remap::DrawMiddle(float& off) {
	
}

void Node_Remap::LoadOut(const std::string& path) {
    Execute();
}