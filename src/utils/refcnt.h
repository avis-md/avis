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
#include "Engine.h"

class RefCnt {
protected:
	RefCnt() {
		_ptr = std::make_shared<char>(' ');
	}
	RefCnt(RefCnt const& rhs) : _ptr(rhs._ptr) {}
	RefCnt& operator= (RefCnt const& rhs) {
		CheckUniqueRef();
		_ptr = rhs._ptr;
		return *this;
	}
	void CheckUniqueRef() {
		if (_ptr.unique()) DestroyRef();
	}
	virtual void DestroyRef() = 0;
private:
	std::shared_ptr<char> _ptr;
};