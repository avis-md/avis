#include "web/anweb.h"

std::unordered_map<std::string, std::weak_ptr<FScript>> FScript::allScrs;

FScript::FScript() : AnScript(TYPE::FORTRAN), lib(nullptr), funcLoc(nullptr), pre(-1), post(-1), 
	arr_in_shapeloc(nullptr), arr_in_dataloc(nullptr), arr_out_shapeloc(nullptr), arr_out_dataloc(nullptr){}

void FScript::Clear() {
	AnScript::Clear();
	_inputs.clear();
	_outputs.clear();
	_inarr_pre.clear();
	_outarr_post.clear();
}

pAnScript_I FScript::CreateInstance() {
	auto res = std::make_shared<FScript_I>();
	res->Init(this);
	return res;
}

FScript_I::~FScript_I() {}

void* FScript_I::Resolve(uintptr_t o) {
	return (void*)o;
}

int* FScript_I::GetDimValue(const CVar::szItem& i) {
	return i.useOffset ? (int*)Resolve(i.offset) : (int*)&i.size;
}

#define CS_SET(t) void FScript_I::SetInput(int i, t val) {\
	auto scr = ((FScript*)parent);\
	*(t*)Resolve(scr->_inputs[i].offset) = val;\
}

CS_SET(short)

CS_SET(int)

CS_SET(double)

void FScript_I::SetInput(int i, void* val, char tp, std::vector<int> szs) {
	auto scr = ((FScript*)parent);
	auto& iv = ((FScript*)parent)->_inputs[i];
	*scr->arr_in_shapeloc = szs.data();
	*scr->arr_in_dataloc = val;
	scr->_inarr_pre[i]();
}

void FScript_I::GetOutput(int i, int* val) {
	auto scr = ((FScript*)parent);
	*val = *(int*)Resolve(scr->_inputs[i].offset);
}

void FScript_I::GetOutputArrs() {
	auto scr = ((FScript*)parent);
	for (uint i = 0; i < scr->outputs.size(); ++i) {
		scr->post = i;
		auto& vr = scr->outputs[i];
		auto& or = outputArrs[i];
		if (vr.type == AN_VARTYPE::LIST) {
			scr->_outarr_post[i]();
			int sz = vr.stride;
			for (size_t a = 0; a < vr.dim; ++a) {
				sz *= (or.dims[a] = (*scr->arr_out_shapeloc)[a]);
			}
			or.val.arr.resize(sz);
			or.val.pval = or.val.arr.data();
			memcpy(or.val.pval, *scr->arr_out_dataloc, sz);
		}
	}
	scr->post = -1;
}

float FScript_I::GetProgress() {
	return AnScript_I::GetProgress();
}