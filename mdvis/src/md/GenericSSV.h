#pragma once
#include "Engine.h"
#include "importer_info.h"

class GenericSSV {
	enum class TYPES : byte {
		NONE, ID, RID, RNM, TYP,
		POSX, POSY, POSZ,
		VELX, VELY, VELZ,
		ATTR
	};
	typedef std::vector<std::vector<double>> vecvecd;
public:
	static bool Read(ParInfo* info);
	static bool ReadFrm(FrmInfo* info);

	typedef std::pair<std::string, std::vector<double>> AttrTyp;
	static std::vector<AttrTyp> attrs;
	static vecvecd _attrs;
	static std::vector<vecvecd> _attrsf;
private:
	static std::vector<TYPES> _tps;
	static std::string _s;

	static void ParseTypes(const std::string& line, std::vector<TYPES>& ts);
};