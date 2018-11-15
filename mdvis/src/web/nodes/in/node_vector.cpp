#include "node_vector.h"

Node_Vector::Node_Vector() : AnNode(new DmScript(sig)) {
    title = "Set Bounding Box Center";
    titleCol = NODE_COL_NRM;
    inputR.resize(3);
    auto& vrs = script->invars;
    script->invaropts.resize(3);
	vrs.resize(3, std::pair<std::string, std::string>("X", "double"));
	vrs[1].first = "Y";
	vrs[2].first = "Z";

	outputR.resize(1);
	conV.resize(1);
	conVAll.resize(1);
	script->outvars.resize(1, std::pair<std::string, std::string>("vector", "list(1d)"));
	conV[0].data.dims.resize(1, 3);
	conV[0].dimVals.resize(1, &conV[0].data.dims[0]);
	conV[0].data.val.arr.p = &vec;
	conV[0].value = &conV[0].data.val.arr.p;
}

void Node_Vector::Execute() {
	vec.x = *(double*)inputR[0].getval();
	vec.y = *(double*)inputR[1].getval();
	vec.z = *(double*)inputR[2].getval();
}