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
#include "web/annode_internal.h"

class Node_AdjList : public AnNode {
public:
	INODE_DEF_H
	Node_AdjList();

	void Execute() override;

protected:
	std::vector<int> conns;
	int count, listsize;
};

class Node_AdjListI : public AnNode {
public:
	INODE_DEF_H
	Node_AdjListI();

	void Execute() override;

protected:
	std::vector<int> conns;
	int count;
};