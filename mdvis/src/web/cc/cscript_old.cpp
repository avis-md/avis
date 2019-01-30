#include "anweb.h"

CVar::CVar(std::string nm, AN_VARTYPE tp) : name(nm), type(tp) {
	switch (tp) {
	case AN_VARTYPE::SHORT:
		typeName = "short";
		break;
	case AN_VARTYPE::INT:
		typeName = "int";
		break;
	case AN_VARTYPE::DOUBLE:
		typeName = "double";
		break;
	default:
		break;
	}
}

CVar::CVar(std::string nm, char tp, int dim, std::initializer_list<int*> szs, std::initializer_list<int> defszs) : 
		name(nm), typeName("list(xx)"), type(AN_VARTYPE::LIST) {
	typeName[5] = '0' + dim;
	typeName[6] = tp;
	data.dims.resize(dim);
	dimVals.resize(dim);
	auto df = defszs.begin();
	for (int a = 0; a < dim; ++a) {
		const auto& p = (szs.size() > a)? szs.begin()[a] : nullptr;
		if (p) dimVals[a] = p;
		else {
			if (df < defszs.end()) data.dims[a] = *(df++);
			dimVals[a] = &data.dims[a];
		}
	}

	switch (tp) {
	case 's':
		stride = 2;
		break;
	case 'i':
		stride = 4;
		break;
	case 'd':
		stride = 8;
		break;
	default:
		Debug::Error("CVar", "Unexpected type " + std::string(&tp, 1) + "!");
		break;
	}
}

CVar::CVar(const CVar& cv) {
	name = cv.name;
	typeName = cv.typeName;
	type = cv.type;
	data = cv.data;
	if (type == AN_VARTYPE::LIST && cv.data.val.arr.data.size() > 0 && cv.data.val.arr.p == cv.data.val.arr.data.data()) {
		data.val.arr.p = data.val.arr.data.data();
	}
	if (cv.value == &cv.data.val.arr.p) {
		value = data.val.arr.p;
	}
	else value = cv.value;
	dimNames = cv.dimNames;
	dimVals = cv.dimVals;
	for (size_t a = 0; a < dimVals.size(); ++a) {
		if (cv.dimVals[a] == &cv.data.dims[a]) {
			dimVals[a] = &data.dims[a];
		}
	}
	stride = cv.stride;
}

CVar& CVar::operator= (const CVar& cv) {
	name = cv.name;
	typeName = cv.typeName;
	type = cv.type;
	std::memcpy(&data, &cv.data, sizeof(data));
	if (type == AN_VARTYPE::LIST && cv.data.val.arr.data.size() > 0 && cv.data.val.arr.p == cv.data.val.arr.data.data()) {
		data.val.arr.p = data.val.arr.data.data();
	}
	if (cv.value == &cv.data.val.arr.p) {
		value = data.val.arr.p;
	}
	else value = cv.value;
	dimNames = cv.dimNames;
	dimVals = cv.dimVals;
	for (size_t a = 0; a < dimVals.size(); ++a) {
		if (cv.dimVals[a] == &cv.data.dims[a]) {
			dimVals[a] = &data.dims[a];
		}
	}
	stride = cv.stride;
	return *this;
}

void CVar::Write(std::ofstream& strm) {
	_StreamWrite(&type, &strm, 1);
	switch (type) {
	case AN_VARTYPE::DOUBLE:
		_StreamWrite((double*)value, &strm, 8);
		break;
	case AN_VARTYPE::INT:
		_StreamWrite((int32_t*)value, &strm, 4);
		break;
	case AN_VARTYPE::LIST:
		{
			int totalSz = 1;
			auto sz = dimVals.size();
			_StreamWrite(&sz, &strm, 1);
			for (uint a = 0; a < sz; ++a) {
				_StreamWrite((int32_t*)dimVals[a], &strm, 4);
				totalSz *= *dimVals[a];
			}
			if (totalSz > 0) {
				auto po = strm.tellp();
				strm.write(*((char**)value), totalSz * stride);
				int wt = (int)(strm.tellp() - po);
				if (wt < totalSz * stride || strm.bad()) {
					Debug::Error("CVar", "not enough bytes written!");
				}
			}
		}
		break;
	default:
		Debug::Warning("CVar", "write case not handled!");
		break;
	}
}

void CVar::Read(std::ifstream& strm) {
	_Strm2Val(strm, type);
	switch (type) {
	case AN_VARTYPE::DOUBLE:
		value = &data.val.d;
		_Strm2Val(strm, data.val.d);
		break;
	case AN_VARTYPE::INT:
		value = &data.val.i;
		_Strm2Val(strm, (int32_t&)data.val.i);
		break;
	case AN_VARTYPE::LIST:
		{
			byte sz = 0;
			_Strm2Val(strm, sz);
			dimVals.resize(sz);
			data.dims.resize(sz);
			int totalSz = 1;
			for (auto a = 0; a < sz; ++a) {
				dimVals[a] = &data.dims[a];
				_Strm2Val(strm, data.dims[a]);
				totalSz *= data.dims[a];
			}
			data.val.arr.data.resize(std::max(totalSz * stride, 1));
			data.val.arr.p = &data.val.arr.data[0];
			value = &data.val.arr.p;
			if (!!totalSz) {
				strm.read(&data.val.arr.data[0], totalSz * stride);
				if (strm.gcount() != totalSz * stride || strm.bad()) {
					Debug::Error("CVar", "not enough bytes read!");
				}
			}
		}
		break;
	default:
		Debug::Warning("CVar", "read case not handled!");
		break;
	}
}


std::unordered_map<std::string, CScript*> CScript::allScrs;

bool CScript::Clear() {
	AnScript::Clear();
	_invars.clear();
	_outvars.clear();
	if (AnWeb::hasC) {
		if (!DyLib::ForceUnload(lib, libpath))
			return false;
		lib = DyLib();
	}
	return true;
}

std::string CScript::Exec() {
#ifdef PLATFORM_WIN
	if (CReader::useMsvc) {
		funcLoc();
	}
	else {
		auto res = wFuncLoc();
		if (res)
			throw res;
	}
#else
	funcLoc();
#endif
	return "";
}
