#include "node_adjlist.h"

#define RETERR(msg) { std::cerr << msg << std::endl; return; }

Node_AdjList::Node_AdjList() : AnNode(new DmScript(sig)) {
	auto scr = (DmScript*)script;
	title = "To Adjacency List";
	titleCol = Vec3(0.3f, 0.5f, 0.3f);
	
	AddInput();
	script->AddInput("pairlist", "list(2i)");
	AddInput();
	script->AddInput("id count", "int");
	AddInput();
	script->AddInput("capacity", "int");

	AddOutput(CVar("adjlist", 'i', 2, { &count, &listsize }));
	auto& cv = conV[0];
	script->AddOutput(cv);
	cv.value = &cv.data.val.arr.p;
}

void Node_AdjList::Execute() {
	if (!inputR[0].first) return;
	auto& cv = inputR[0].getconv();
	auto data = *(int**)cv.value;
	auto ccnt = *cv.dimVals[0];
	count = inputR[1].first? *(int*)inputR[1].getval() : inputVDef[1].i;
	listsize = inputR[2].first? *(int*)inputR[2].getval() : inputVDef[2].i;
	if (listsize <= 0) RETERR("list size must be > 0!");
	if (count <= 0) RETERR("atom count is 0!");
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
	conV[0].data.val.arr.p = &conns[0];
}


Node_AdjListI::Node_AdjListI() : AnNode(new DmScript(sig)) {
	auto scr = (DmScript*)script;
	title = "To Paired List";
	titleCol = NODE_COL_NRM;

	AddInput();
	script->AddInput("adjlist", "list(2i)");

	AddOutput(CVar("pairlist", 'i', 2, { &count, nullptr }, { 2 }));
	auto& cv = conV[0];
	script->AddOutput(cv);
	auto v = *conV[0].dimVals[1];
	cv.value = &cv.data.val.arr.p;
}

void Node_AdjListI::Execute() {
	count = 0;
	auto v = *conV[0].dimVals[1];
	if (!inputR[0].first) return;
	auto& cv = inputR[0].first->conV[inputR[0].second];
	auto data = *(int**)cv.value;
	auto cnt = *cv.dimVals[0];
	auto lsz = *cv.dimVals[1];
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
	conV[0].data.val.arr.p = &conns[0];
}