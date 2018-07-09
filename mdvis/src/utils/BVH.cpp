#include "BVH.h"
#include <queue>

struct _calc_st {
	_calc_st(byte b) : axis(b) {}
	uint* ids;
	uint idc;
	byte axis;
	uint* _idv = 0;
};

void BVH::Calc(Ball* objs, uint cnt, Node*& res, uint& resCnt, BBox box) {
	if (cnt < 3) return;

	res = new Node[cnt * 2];

	std::queue<_calc_st> que;

	que.push(_calc_st(1));
	auto& bk1 = que.back();
	que.push(_calc_st(1));
	auto& bk2 = que.back();
	auto& vc = bk2._idv;
	vc = new uint[cnt];

	uint i1 = 0;
	uint i2 = cnt;
	box.x0 -= 1000; box.y0 -= 1000; box.z0 -= 1000;
	box.x1 += 1000; box.y1 += 1000; box.z1 += 1000;
	float md = (box.x0 + box.x1) / 2;
	for (uint a = 0; a < cnt; a++) {
		if (objs[a].orig.x > md) vc[--i2] = a;
		else vc[i1++] = a;
	}
	bk1.ids = &vc[0];
	bk1.idc = i1;
	bk2.ids = &vc[i2];
	bk2.idc = cnt - i2;

	res->box = box;
	res->val1 = 1;
	res->val2 = 2;

	Node* nd = res + 1;
	uint ndl = 3;
	resCnt = 1;
	while (!que.empty()) {
		_calc_st& st = que.front();
		if (st.idc < 3) {
			nd->box.x0 = 1;
			nd->box.x1 = 0;
			nd->val1 = st.ids[0];
			if (st.idc == 1) {
				nd->box.y0 = -1;
				nd->val2 = -1;
			}
			else {
				nd->box.y0 = 1;
				nd->val2 = st.ids[1];
			}
		}
		else {
			Ball* bl = objs + st.ids[0];
			nd->box.x0 = bl->orig.x - bl->rad; nd->box.x1 = bl->orig.x + bl->rad;
			nd->box.y0 = bl->orig.y - bl->rad; nd->box.y1 = bl->orig.y + bl->rad;
			nd->box.z0 = bl->orig.z - bl->rad; nd->box.z1 = bl->orig.z + bl->rad;

			byte ax2 = (st.axis + 1) % 3;
			que.push(_calc_st(ax2));
			auto& bk1 = que.back();
			que.push(_calc_st(ax2));
			auto& bk2 = que.back();
			auto& vc = bk2._idv;

			uint i1 = 0;
			uint i2 = st.idc;
			vc = new uint[i2];

			if (resCnt == 5653) {
				int i = 1;
				i++;
			}

			float md2 = bl->orig[st.axis];//((st.axis == 0) ? bl.orig.x : ((st.axis == 1) ? bl.orig.y : bl.orig.z));
			for (int a = 1; a < st.idc; a++) {
				bl = objs + st.ids[a];
#define MN(a, b) a = min(a, b)
#define MX(a, b) a = max(a, b)
				MN(nd->box.x0, bl->orig.x - bl->rad);
				MN(nd->box.y0, bl->orig.y - bl->rad);
				MN(nd->box.z0, bl->orig.z - bl->rad);
				MX(nd->box.x1, bl->orig.x + bl->rad);
				MX(nd->box.y1, bl->orig.y + bl->rad);
				MX(nd->box.z1, bl->orig.z + bl->rad);
				md2 += bl->orig[st.axis];
			}
			md2 /= st.idc;
			//float md = ((st.axis == 0) ? nd->box.x0 + nd->box.x1 : ((st.axis == 1) ? nd->box.y0 + nd->box.y1 : nd->box.z0 + nd->box.z1)) / 2;
			for (int a = 0; a < st.idc; a++) {
				uint i = st.ids[a];
				bl = objs + i;
				float orr = bl->orig[st.axis];
				if (orr > md2) vc[--i2] = i;
				else vc[i1++] = i;
			}
			if (i1 == 0 || i2 == st.idc) {
				i1 = st.idc/2;
				i2 = st.idc - i1;
			}
			bk1.ids = &vc[0];
			bk1.idc = i1;
			bk2.ids = &vc[i2];
			bk2.idc = st.idc - i2;
			nd->val1 = ndl++;
			nd->val2 = ndl++; //always val1 + 1?
		}

		delete[](st._idv);
		que.pop();
		nd++;
		resCnt++;
	}
}