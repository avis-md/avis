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

/*! Base class of instantiatable object
[av] */
class Object : public std::enable_shared_from_this<Object> {
public:
	Object(std::string nm = "");
	Object(ulong id, std::string nm);
	ulong id;
	std::string name;
	bool dirty = false; //triggers a reload of internal variables
	bool dead = false; //will be cleaned up after this frame

	virtual bool ReferencingObject(Object* o) { return false; }
};