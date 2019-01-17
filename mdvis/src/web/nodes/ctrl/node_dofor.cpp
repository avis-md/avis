#include "node_dofor.h"
#include "web/anweb.h"

INODE_DEF("Do For", DoFor, MISC);

int Node_DoFor::_i = -1, Node_DoFor::_im = 0, Node_DoFor::_sz = 0;
uint Node_DoFor::_st = 0;

Node_DoFor::Node_DoFor() : INODE_INIT {
	INODE_TITLE(NODE_COL_SPC);

	AddInput();
	scr.AddInput("arr", "list(2d)");
	AddOutput(CVar("comp", 'd', 1, {&_sz}, {}));
	scr.AddOutput(conV.back());
	conV[0].value = &conV[0].data.val.arr.p;
}

void Node_DoFor::Execute() {
	auto& cv = inputR[0].getconv();
	auto& cvo = conV[0];

	if (_i == -1) {
		_sz = *cv.dimVals[1];
		_im = *cv.dimVals[0];
		cvo.data.val.arr.data.resize(_sz * cv.stride);
		cvo.data.val.arr.p = cvo.data.val.arr.data.data();
		_st = AnWeb::currNode;
	}
	_i++;
	const auto& src = *((double**)cv.value);
	double te[10];
	memcpy(te, src, 6 * sizeof(double));
	memcpy(cvo.data.val.arr.data.data(), src + _i * _sz, _sz * cv.stride);
}



INODE_DEF("Do For (End)", DoForEnd, MISC);

Node_DoForEnd::Node_DoForEnd() : INODE_INIT {
	INODE_TITLE(NODE_COL_SPC);
}

void Node_DoForEnd::Execute() {
	if (Node_DoFor::_im > Node_DoFor::_i + 1) {
		AnWeb::nextNode = Node_DoFor::_st;
	}
	else Node_DoFor::_i = -1;
}