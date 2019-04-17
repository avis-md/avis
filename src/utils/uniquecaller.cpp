#include "uniquecaller.h"

bool UniqueCaller::operator== (const UniqueCaller& rhs) {
	for (int a = 0; a < UI_MAX_EDIT_TEXT_FRAMES; a++) {
		if (frames[a] != rhs.frames[a]) return false;
	}
	return id == rhs.id;
}

void UniqueCallerList::Preloop() {
	frames.clear();
}

bool UniqueCallerList::Add() {
	current = UniqueCaller();
	Debug::StackTrace(UI_MAX_EDIT_TEXT_FRAMES, (void**)current.frames.data());
	current.id = ++frames[current.frames];
	return current == last;
}

void UniqueCallerList::Set() {
	last = current;
}

void UniqueCallerList::Clear() {
	last = UniqueCaller();
}