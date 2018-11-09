#include "anweb.h"

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
		delete(lib);
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
