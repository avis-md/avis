#include "GenericSSV.h"

#define SETERR(msg) memcpy(info->error, msg, sizeof(msg))

std::vector<GenericSSV::AttrTyp> GenericSSV::attrs;
std::vector<std::vector<float>> GenericSSV::_attrs;
std::vector<GenericSSV::TYPES> GenericSSV::_tps;
std::string GenericSSV::_s;

bool GenericSSV::Read(ParInfo* info) {
	std::ifstream strm(PATH(info->path));
	std::string p;
	std::getline(strm, p);
	_s = p;
	std::vector<TYPES> ts;
	attrs.clear();
	_attrs.clear();
	ParseTypes(p, ts);
	if (!ts.size()) {
		SETERR("No types specified!");
		return false;
	}
	_tps = ts;

	strm >> info->num;
	auto& sz = info->num;
	if (!sz) {
		SETERR("Atom count is zero or not available!");
		return false;
	}

	info->resId = new uint16_t[sz]{};
	info->resname = new char[sz * info->nameSz]{};
	info->name = new char[sz * info->nameSz]{};
	info->type = new uint16_t[sz]{};
	info->pos = new double[sz * 3]{};
	info->vel = new double[sz * 3]{};
	
	for (auto& a : attrs) {
		a.second.resize(sz);
	}
	for (int a = 0; a < sz; ++a) {
		int attri = 0;
		for (auto& t : ts) {
			strm >> p;
			switch (t) {
			case TYPES::RID:
				info->resId[a] = (uint16_t)std::stod(p);
				break;
			case TYPES::RNM:
				p.copy(info->resname + a*info->nameSz, info->nameSz);
				break;
			case TYPES::TYP:
				p.copy(info->name + a*info->nameSz, info->nameSz);
				info->type[a] = p[0];
				break;
			case TYPES::POSX: info->pos[a*3] = std::stod(p); break;
			case TYPES::POSY: info->pos[a*3 + 1] = std::stod(p); break;
			case TYPES::POSZ: info->pos[a*3 + 2] = std::stod(p); break;
			case TYPES::VELX: info->vel[a*3] = std::stod(p); break;
			case TYPES::VELY: info->vel[a*3 + 1] = std::stod(p); break;
			case TYPES::VELZ: info->vel[a*3 + 2] = std::stod(p); break;
			case TYPES::ATTR: attrs[attri++].second[a] = std::stof(p); break;
			default: break;
			}
		}
		strm.ignore(1000, '\n');
	}

	auto& bnd = info->bounds;
	bnd[0] = bnd[1] = (float)info->pos[0];
	bnd[2] = bnd[3] = (float)info->pos[1];
	bnd[4] = bnd[5] = (float)info->pos[2];
	for (uint32_t i = 1; i < sz; ++i)  {
		bnd[0] = std::min(bnd[0], info->pos[i * 3]);
		bnd[1] = std::max(bnd[1], info->pos[i * 3]);
		bnd[2] = std::min(bnd[2], info->pos[i * 3 + 1]);
		bnd[3] = std::max(bnd[3], info->pos[i * 3 + 1]);
		bnd[4] = std::min(bnd[4], info->pos[i * 3 + 2]);
		bnd[5] = std::max(bnd[5], info->pos[i * 3 + 2]);
	}
	return true;
}

bool GenericSSV::ReadFrm(FrmInfo* info) {
	std::ifstream strm(PATH(info->path));
	std::string p;
	std::getline(strm, p);
	if (p != _s) {
		SETERR("Header contents are different!");
		return false;
	}
	uint32_t psz;
	strm >> psz;
	if (psz != info->parNum) {
		SETERR("Atom count is different!");
		return false;
	}
	
	for (int a = 0; a < psz; ++a) {
		int attri = 0;
		for (auto& t : _tps) {
			strm >> p;
			switch (t) {
			case TYPES::POSX: info->pos[a*3] = std::stod(p); break;
			case TYPES::POSY: info->pos[a*3 + 1] = std::stod(p); break;
			case TYPES::POSZ: info->pos[a*3 + 2] = std::stod(p); break;
			case TYPES::VELX: info->vel[a*3] = std::stod(p); break;
			case TYPES::VELY: info->vel[a*3 + 1] = std::stod(p); break;
			case TYPES::VELZ: info->vel[a*3 + 2] = std::stod(p); break;
			case TYPES::ATTR: {
					auto& at = _attrs[attri++];
					at.resize(info->parNum);
					at[a] = std::stod(p);
					break;
				}
			default: break;
			}
		}
		strm.ignore(1000, '\n');
	}

	return true;
}

void GenericSSV::ParseTypes(const std::string& line, std::vector<GenericSSV::TYPES>& ts) {
	if (line[0] != '#') return;
	auto lns = string_split(line, ' ', true);
	lns[0] = lns[0].substr(1);
	if (lns[0] == "") lns.erase(lns.begin());
#define IS(s, t) if (a == #s) {\
			ts.push_back(TYPES::t);\
		}
	for (auto& a : lns) {
		IS(id, ID)
		else IS(resid, RID)
		else IS(resnm, RNM)
		else IS(type, TYP)
		else IS(posx, POSX)
		else IS(posy, POSY)
		else IS(posz, POSZ)
		else IS(velx, VELX)
		else IS(vely, VELY)
		else IS(velz, VELZ)
		else if (a.size() > 5 && a.substr(0, 5) == "attr=") {
			ts.push_back(TYPES::ATTR);
			attrs.push_back(AttrTyp());
			attrs.back().first = a.substr(5);
			_attrs.push_back(std::vector<float>());
		}
		else {
			ts.push_back(TYPES::NONE);
		}
	}
}