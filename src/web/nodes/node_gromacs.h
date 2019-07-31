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
#include "node_inputs.h"

class Node_Gromacs : public Node_Inputs {
public:
	static const std::string sig;
	Node_Gromacs();

	std::string file;

	Vec2 DrawConn() override {
		return Vec2(width, 36 + 17 * 4);
	}
	void Draw() override;
	void Execute() override;

	void SaveIn(const std::string& path) override {}
	void LoadIn(const std::string& path) override {}
};