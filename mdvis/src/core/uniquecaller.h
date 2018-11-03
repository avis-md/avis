#include "Engine.h"

class UniqueCaller {
public:
	UniqueCaller() : frames({}), id(-1) {}
private:
	std::array<size_t, UI_MAX_EDIT_TEXT_FRAMES> frames;
	int id;
};

class UniqueCallee {
public:
	bool IsActive(UniqueCaller&);
private:
	std::unordered_map<std::array<size_t, UI_MAX_EDIT_TEXT_FRAMES>, int> callers;
};