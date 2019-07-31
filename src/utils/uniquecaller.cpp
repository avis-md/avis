// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

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