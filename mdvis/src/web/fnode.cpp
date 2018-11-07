#include "anweb.h"
#include "anconv.h"
#ifndef IS_ANSERVER
#include "ui/icons.h"
#include "ui/ui_ext.h"
#include "res/resdata.h"
#endif

FNode::FNode(FScript* scr) : AnNode(scr) {
	if (!scr) return;
	title += " (fortran)";
	inputV.resize(scr->invars.size());
	outputV.resize(scr->outvars.size());
	inputVDef.resize(scr->invars.size());
	for (uint i = 0; i < scr->invars.size(); ++i) {
		inputV[i] = scr->_invars[i].value;
		if (scr->_invars[i].type == AN_VARTYPE::DOUBLE) inputVDef[i].d = 0;
		else inputVDef[i].i = 0;
	}
	for (uint i = 0; i < scr->outvars.size(); ++i) {
		outputV[i] = scr->_outvars[i].value;
		conV[i] = scr->_outvars[i];
	}
}

void FNode::Execute() {
	auto scr = (FScript*)script;
	for (uint i = 0; i < scr->invars.size(); ++i) {
		scr->pre = i;
		auto& mv = scr->_invars[i];
		if (HasConnI(i)) {
			auto& cv = inputR[i].first->conV[inputR[i].second];
			switch (mv.type) {
			case AN_VARTYPE::INT:
				scr->Set(i, *(int*)cv.value);
				break;
			case AN_VARTYPE::DOUBLE:
				scr->Set(i, *(double*)cv.value);
				break;
			case AN_VARTYPE::LIST:
			{
				auto nd = cv.dimVals.size();
				int32_t* dims = new int32_t[nd];
				for (size_t a = 0; a < nd; a++) dims[nd - a - 1] = *cv.dimVals[a];
				*scr->arr_in_shapeloc = dims;
				*scr->arr_in_dataloc = *(void**)cv.value;
				scr->_inarr_pre[i]();
				delete[](dims);
				break;
			}
			default:
				OHNO("CNode", "Unexpected scr_vartype " + std::to_string((int)(mv.type)));
				throw "";
			}
		}
		else {
			switch (mv.type) {
			case AN_VARTYPE::INT:
				scr->Set(i, inputVDef[i].i);
				break;
			case AN_VARTYPE::DOUBLE:
				scr->Set(i, inputVDef[i].d);
				break;
			case AN_VARTYPE::LIST:
				throw("Input variable not set!\1");
			default:
				OHNO("CNode", "Unexpected scr_vartype " + std::to_string((int)(mv.type)));
				throw "";
			}
		}
	}
	scr->pre = -1;
	scr->post = -1;
	
	scr->Exec();

	for (uint i = 0; i < scr->outvars.size(); ++i) {
		scr->post = i;
		if (scr->_outvars[i].type == AN_VARTYPE::LIST) {
			scr->_outarr_post[i]();
			auto& cv = conV[i];
			size_t dn = cv.dimVals.size();
			cv.data.dims.resize(dn);
			int sz = 1;
			for (size_t a = 0; a < dn; ++a) {
				cv.data.dims[a] = (*scr->arr_out_shapeloc)[a];
				cv.dimVals[a] = &cv.data.dims[a];
				sz *= cv.data.dims[a];
			}
			cv.data.val.arr.data.resize(cv.stride * sz);
			cv.data.val.arr.p = &cv.data.val.arr.data[0];
			memcpy(cv.data.val.arr.p, *scr->arr_out_dataloc, cv.stride * sz);
			cv.value = &cv.data.val.arr.p;
		}
	}
	scr->post = -1;
}

void FNode::WriteFrame(int f) {

}

void FNode::RemoveFrames() {
	AnNode::RemoveFrames();
	auto scr = (FScript*)script;
	for (uint i = 0; i < scr->outvars.size(); ++i) {
		conV[i] = scr->_outvars[i];
	}
}

void FNode::CatchExp(char* c) {
	std::string s = c;
	if (s.back() == -1) {
		AnNode::CatchExp(c);
		return;
	}
	ErrorView::Message msg{};
	msg.name = script->name;
	msg.path = script->path;
	msg.severe = true;
	if (string_find(s, "At line ") == 0) {
		if ((msg.linenum = atoi(c + 8)) > 0) {
			auto lc = strchr(c, '\n');
			log.push_back(std::pair<byte, std::string>(2, lc + 1));
			std::string s(c + 9, (size_t)lc - (size_t)c - 20); //_temp__.f90\n
			msg.msg.resize(1, lc + 1);
			msg.msg.push_back("Fortran runtime error caught by handler");
			ErrorView::execMsgs.push_back(msg);
			return;
		}
	}
	else if (s.back() == 1) {
		s.pop_back();
		log.push_back(std::pair<byte, std::string>(2, s));
		msg.msg.resize(1, s);
		auto scr = (FScript*)script;
		if (scr->pre > -1) {
			msg.msg.push_back("While handling input variable " + scr->invars[scr->pre].first);
		}
		else if (scr->post > -1) {
			msg.msg.push_back("While handling output variable " + scr->outvars[scr->post].first);
		}
		ErrorView::execMsgs.push_back(msg);
		return;
	}
	AnNode::CatchExp(c);
}