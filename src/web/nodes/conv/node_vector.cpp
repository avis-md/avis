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