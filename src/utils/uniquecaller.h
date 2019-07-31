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

#pragma once
class UniqueCaller;
class UniqueCallerList;

#include "Engine.h"

namespace std
{
    template<typename T, size_t N>
    struct hash<array<T, N> >
    {
        typedef array<T, N> argument_type;
        typedef size_t result_type;

        result_type operator()(const argument_type& a) const
        {
            hash<T> hasher;
            result_type h = 0;
            for (result_type i = 0; i < N; ++i)
            {
                h = h * 31 + hasher(a[i]);
            }
            return h;
        }
    };
}
typedef std::array<uintptr_t, UI_MAX_EDIT_TEXT_FRAMES> traceframes;

class UniqueCaller {
public:
	traceframes frames;
	ushort id;

	bool operator== (const UniqueCaller& rhs);
};

class UniqueCallerList {
public:
	std::unordered_map<traceframes, ushort> frames;

	UniqueCaller current, last;

	void Preloop();
	bool Add();
	void Set(), Clear();
};