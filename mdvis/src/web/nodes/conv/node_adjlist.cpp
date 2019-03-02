#include "node_adjlist.h"
#include "web/anweb.h"

#define RETERR(msg) { std::cerr << msg << std::endl; return; }

INODE_DEF(__("To Adjacency List"), AdjList, CONV)

Node_AdjList::Node_AdjList() : INODE_INIT {
	INODE_TITLE(NODE_COL_NRM);
	INODE_SINIT(
		scr->desc = R"(Converts a pair list into
an adjacency list)";
		scr->descLines = 2;

		scr->AddInput(_("list"), AN_VARTYPE::INT, 2);
		scr->AddInput(_("max id"), AN_VARTYPE::INT);
		scr->AddInput(_("capacity"), AN_VARTYPE::INT);

		scr->AddOutput(_("result"), AN_VARTYPE::INT, 2);
	);

	IAddConV(0, { &count, &listsize }, {});

}

void Node_AdjList::Execute() {
	if (!inputR[0].first) return;
	if (*inputR[0].getdim(1) != 2)
		RETERR("input list dim 2 size is not 2!");
	auto data = *(int**)inputR[0].getval();
	auto ccnt = *inputR[0].getdim(0);
	count = getval_i(1);
	listsize = getval_i(2);
	if (listsize <= 0) RETERR("list size must be positive!");
	if (count <= 0) RETERR("max id must be positive!");
	conns.clear();
	conns.resize(count*listsize, -1);
	for (int a = 0; a < ccnt; ++a) {
		int i0 = data[a*2];
		int i1 = data[a*2+1];
		int* p0 = &conns[i0*listsize];
		int* p1 = &conns[i1*listsize];
		for (int c = 0; c < listsize; ++c) {
			if (*p0 == -1) break;
			p0++;
		}
		if (*p0 != -1)
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
	auto data = *(int**)ir.getval();
	auto cnt = *ir.getdim(0);
	auto lsz = *ir.getdim(1);
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