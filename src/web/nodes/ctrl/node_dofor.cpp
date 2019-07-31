// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

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
	AddOutput(CVar("index", AN_VARTYPE::INT));
	scr.AddOutput(conV.back());
	AddOutput(CVar("count", AN_VARTYPE::INT));
	scr.AddOutput(conV.back());
	conV[0].value = &conV[0].data.val.arr.p;
	conV[1].value = &_i;
	conV[2].value = &_im;
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