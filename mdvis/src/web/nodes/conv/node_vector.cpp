#include "node_vector.h"

INODE_DEF("To Vector", ToVector, CONV);

Node_ToVector::Node_ToVector() : INODE_INIT {
	INODE_TITLE(NODE_COL_NRM);
	
	AddInput();
	scr.AddInput("X", "double");
	AddInput();
	scr.AddInput("Y", "double");
	AddInput();
	scr.AddInput("Z", "double");

	AddOutput();
	scr.AddOutput(CVar("result", 'd', 1, {}, {3}));

	conV[0].data.val.arr.p = &vec;
	conV[0].value = &conV[0].data.val.arr.p;
}

void Node_ToVector::Execute() {
	vec.x = getval_d(0);
	vec.y = getval_d(1);
	vec.z = getval_d(2);
}