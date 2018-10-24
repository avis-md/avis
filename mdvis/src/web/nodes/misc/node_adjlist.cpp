#include "node_adjlist.h"

#define RETERR(msg) { std::cerr << msg << std::endl; return; }

Node_AdjList::Node_AdjList() : AnNode(new DmScript(sig)) {
	auto scr = (DmScript*)script;
	title = "To Adjacency List";
	titleCol = Vec3(0.3f, 0.5f, 0.3f);
	inputR.resize(3);
	scr->invaropts.resize(3);
	scr->invars.push_back(std::pair<std::string, std::string>("bonds", "list(2i)"));
	scr->invars.push_back(std::pair<std::string, std::string>("parcount", "int"));
	scr->invars.push_back(std::pair<std::string, std::string>("size", "int"));
	inputVDef.resize(3);

	outputR.resize(1);
	script->outvars.resize(1, std::pair<std::string, std::string>("list", "list(2i)"));
	
	conV.resize(1);
	auto& cv = conV[0];
	cv.type = AN_VARTYPE::LIST;
	cv.typeName = "list(2i)";
	cv.dimVals.push_back(&count);
	cv.dimVals.push_back(&listsize);
	cv.stride = 4;
}

void Node_AdjList::Execute() {
	if (!inputR[0].first) return;
	auto& cv = inputR[0].getconv();
	auto data = *(int**)cv.value;
	auto ccnt = *cv.dimVals[0];
	count = inputR[1].first? *(int*)inputR[1].getval() : inputVDef[1].i;
	listsize = inputR[2].first? *(int*)inputR[2].getval() : inputVDef[2].i;
	if (!ccnt || !count) return;
	conns.clear();
	conns.resize(count*listsize, -1);
	for (int a = 0; a < ccnt; ++a)  {
		int i0 = data[a*2];
		int i1 = data[a*2+1];
		int* p0 = &conns[i0*listsize];
		int* p1 = &conns[i1*listsize];
		for (int c = 0; c < listsize; ++c)  {
			if (*p0 == -1) break;
			p0++;
		}
		if (*p0 != -1)
			RETERR("List size is too short for index " << i0 << "!");
		for (int c = 0; c < listsize; ++c)  {
			if (*p1 == -1) break;
			p1++;
		}
		if (*p1 != -1)
			RETERR("List size is too short for index " << i1 << "!");
		*p0 = i1; *p1 = i0;
	}
	conV[0].data.val.arr.p = &conns[0];
	conV[0].value = &conV[0].data.val.arr.p;
}


Node_AdjListI::Node_AdjListI() : AnNode(new DmScript(sig)) {
	auto scr = (DmScript*)script;
	title = "To Paired List";
	titleCol = NODE_COL_NRM;
	inputR.resize(1);
	scr->invaropts.resize(1);
	scr->invars.push_back(std::pair<std::string, std::string>("adjlist", "list(2i)"));
	inputVDef.resize(1);

	outputR.resize(1);
	script->outvars.resize(1, std::pair<std::string, std::string>("list", "list(2i)"));
	
	conV.resize(1);
	auto& cv = conV[0];
	cv.type = AN_VARTYPE::LIST;
	cv.typeName = "list(2i)";
	cv.data.dims.resize(2, 2);
	cv.dimVals.push_back(&count);
	cv.dimVals.push_back(&cv.data.dims[1]);
	cv.stride = 4;
}

void Node_AdjListI::Execute() {
	count = 0;
	if (!inputR[0].first) return;
	auto& cv = inputR[0].first->conV[inputR[0].second];
	auto data = *(int**)cv.value;
	auto cnt = *cv.dimVals[0];
	auto lsz = *cv.dimVals[1];
	if (!cnt || !lsz) return;
	conns.clear();
	conns.reserve(cnt*lsz/2);
	for (int a = 0; a < cnt; ++a)  {
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
	conV[0].value = &conV[0].data.val.arr.p;
}