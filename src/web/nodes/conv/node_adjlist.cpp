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

#include "node_adjlist.h"
#include "web/anweb.h"

#define RETERR(msg) { std::cerr << msg << std::endl; return; }

INODE_DEF(__("To Adjacency List"), AdjList, CONV)

Node_AdjList::Node_AdjList() : INODE_INIT {
	INODE_TITLE(NODE_COL_NRM);
	INODE_SINIT(
		scr->desc = R"(Converts a pair list into
an adjacency list
If max id is 0, it is set to
the maximum ID in the data)";
		scr->descLines = 4;

		scr->AddInput(_("list"), AN_VARTYPE::INT, 2);
		scr->AddInput(_("max id"), AN_VARTYPE::INT);
		scr->AddInput(_("capacity"), AN_VARTYPE::INT);

		scr->AddOutput(_("result"), AN_VARTYPE::INT, 2);
	);

	IAddConV(0, { &count, &listsize }, {});

}

void Node_AdjList::Execute() {
	if (!inputR[0].first) return;
	if (inputR[0].getdim(1) != 2)
		RETERR("input list dim 2 size is not 2!");
	auto data = *(int**)inputR[0].getval(ANVAR_ORDER::C);
	auto ccnt = inputR[0].getdim(0);
	count = getval_i(1);
	listsize = getval_i(2);
	if (listsize <= 0) RETERR("list size must be positive!");
	if (count <= 0) {
		count = data[0] + 1;
		for (int a = 1; a < ccnt * 2 - 1; a++) {
			count = std::max(count, data[a] + 1);
		}
	}
	conns.clear();
	conns.resize(count*listsize, -1);
	for (int a = 0; a < ccnt; ++a) {
		int i0 = data[a*2];
		int i1 = data[a*2+1];
		int* p0 = &conns[i0*listsize];
		int* p1 = &conns[i1*listsize];
		int c = 0;
		for (; c < listsize; ++c) {
			if (*p0 == -1) break;
			p0++;
		}
		if (c == listsize)
			RETERR("List size is too short for index " << i0 << "!");
		for (int c = 0; c < listsize; ++c) {
			if (*p1 == -1) break;
			p1++;
		}
		if (*p1 != -1)
			RETERR("List size is too short for index " << i1 << "!");
		*p0 = i1; *p1 = i0;
	}

	auto& ov = ((DmScript_I*)script.get())->outputVs;
	ov[0].val.p = conns.data();
	ov[0].pval = &ov[0].val.p;
}


INODE_DEF(__("To Paired List"), AdjListI, CONV)

Node_AdjListI::Node_AdjListI() : INODE_INIT {
	INODE_TITLE(NODE_COL_NRM);
	INODE_SINIT(
		scr->desc = R"(Converts an adjacency list into
a pair list)";
	scr->descLines = 2;

	scr->AddInput(_("list"), AN_VARTYPE::INT, 2);

	scr->AddOutput(_("result"), AN_VARTYPE::INT, 2);
	);

	IAddConV(0, { &count, 0 }, { 2 });
}

void Node_AdjListI::Execute() {
	count = 0;
	if (!inputR[0].first) return;
	auto& ir = inputR[0];
	auto data = *(int**)ir.getval(ANVAR_ORDER::C);
	auto cnt = ir.getdim(0);
	auto lsz = ir.getdim(1);
	if (!cnt || !lsz) return;
	conns.clear();
	conns.reserve(cnt*lsz/2);
	for (int a = 0; a < cnt; ++a) {
		for (int b = 0; b < lsz; ++b){
			auto c = data[a*lsz+b];
			if (c == -1) break;
			if (c > a) {
				conns.push_back(a);
				conns.push_back(c);
				count++;
			}
		}
	}
	auto& ov = ((DmScript_I*)script.get())->outputVs;
	ov[0].val.p = conns.data();
	ov[0].pval = &ov[0].val.p;
}