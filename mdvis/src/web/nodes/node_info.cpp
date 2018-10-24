#include "node_info.h"
#include "node_inputs.h"
#include "md/particles.h"

Node_Info::Node_Info() :AnNode(new DmScript(sig)) {
	title = "System Info";
	titleCol = NODE_COL_IO;
	canTile = true;
	
	const std::string vars[] = {
		"atom count", "int",
		"frame count", "int",
		"current frame", "int",
		"bounding box", "list(1d)",
	};
	const int sz = sizeof(vars) / sizeof(vars[0]) / 2;
	
	outputR.resize(sz);
	script->outvars.resize(sz);
	conV.resize(sz);

	for (int a = 0; a < sz; ++a)  {
		script->outvars[a] = std::pair<std::string, std::string>(vars[a * 2], vars[a * 2 + 1]);
		conV[a].typeName = vars[a * 2 + 1];
	}
	conV[3].data.dims.resize(1, 6);
	conV[3].dimVals.resize(1, &conV[3].data.dims[0]);
}

void Node_Info::Execute() {
	conV[0].value = &Particles::particleSz;
	conV[1].value = &Particles::anim.frameCount;
	conV[2].value = &Node_Inputs::frame;
	static double* bbx = &Particles::boundingBox[0];
	conV[3].value = &bbx;
}