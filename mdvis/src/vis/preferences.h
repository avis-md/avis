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

public:
	static void Init();

	static bool show;
	static int menu;

	static void Draw();
	static void Reset(), Save(), Load();

	static void Link(const std::string sig, bool* b, void (*callback)() = nullptr);
	static void Link(const std::string sig, int* i, void (*callback)() = nullptr);
	static void Link(const std::string sig, float* f, void (*callback)() = nullptr);
	static void Link(const std::string sig, std::string* s, void (*callback)() = nullptr);
	static void Link(const std::string sig, Vec4* c, void (*callback)() = nullptr);
};