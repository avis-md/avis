#pragma once
#include "system.h"

class Preferences {
	struct Pref {
		std::string sig;
		std::string name;
		std::string desc;
		
		enum class TYPE {
			BOOL, INT, FLOAT, STRING, COLOR
		} type;

		bool minmax, slide;
		int   min_i, max_i;
		float min_f, max_f;
		
		bool changed = false;

		bool        dval_b, *val_b = nullptr;
		int         dval_i, *val_i = nullptr;
		float       dval_f, *val_f = nullptr;
		std::string dval_s, *val_s = nullptr;
		Vec4        dval_c, *val_c = nullptr, val_co;

		void (*callback)();
	};
	static std::vector<std::pair<std::string, std::vector<Pref>>> prefs;

	static bool showre;

	struct Bondlen {
		std::string sig1, sig2;
		float len;
	};
	struct Typecol {
		std::string sig;
		Vec4 col;
	};
	struct Typerad {
		std::string sig;
		float rad;
	};

	static std::vector<Bondlen> bondlengths;
	static std::vector<Typecol> typecolors;
	static std::vector<Typerad> typeradii;

public:
	static void Init();

	static bool show;
	static int menu;

	static void Draw();
	static void Reset(), Save(), Load();
	static void SaveAttrs(), LoadAttrs();
	static void SaveEnv(), LoadEnv();

	static void Link(const std::string sig, bool* b, void (*callback)() = nullptr);
	static void Link(const std::string sig, int* i, void (*callback)() = nullptr);
	static void Link(const std::string sig, float* f, void (*callback)() = nullptr);
	static void Link(const std::string sig, std::string* s, void (*callback)() = nullptr);
	static void Link(const std::string sig, Vec4* c, void (*callback)() = nullptr);

	static float GetLen(ushort sig1, ushort sig2);
	static Vec3 GetCol(ushort sig);
	static float GetRad(ushort sig);

	static void LinkEnv(const std::string sig, std::string* s);
};