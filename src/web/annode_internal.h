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
#include "anweb.h"
#include "ui/localizer.h"

#define INODE_DEF_NOREG_H \
	static const std::string sig;\
	static const char* _name;\
	static std::unique_ptr<DmScript> scr;\
	static std::shared_ptr<AnNode> _Spawn();

#define INODE_DEF_H INODE_DEF_NOREG_H\
	static AnNode_Internal_Reg _reg;

#define INODE_DEF_NOREG(tt, nm) \
	const std::string Node_ ## nm::sig = "." #nm;\
	const char* Node_ ## nm::_name = tt;\
	std::unique_ptr<DmScript> Node_ ## nm::scr = std::unique_ptr<DmScript>(new DmScript(sig));\
	std::shared_ptr<AnNode> Node_ ## nm::_Spawn() { return std::make_shared<Node_ ## nm>(); } 

#define INODE_DEF(tt, nm, gp) INODE_DEF_NOREG(tt, nm) \
	AnNode_Internal_Reg Node_ ## nm::_reg = AnNode_Internal_Reg(\
		Node_ ## nm::sig, Node_ ## nm::_name, ANNODE_GROUP::gp, &Node_ ## nm::_Spawn);

#define INODE_INIT AnNode(scr->CreateInstance(), 0)
#define INODE_INITF(f) AnNode(scr->CreateInstance(), f)

#define INODE_TITLE(col) \
	title = _(_name);\
	titleCol = col;

#define INODE_SINIT(cmd) if (!scr->ok) {\
		cmd\
		scr->ok = true;\
		script->Init(scr.get());\
	}\
	ResizeIO(scr.get());\
	conV.clear();

enum class ANNODE_GROUP : int {
	GET,
	SET,
	GEN,
	CONV,
	MISC,
	_COUNT
};

class AnNode_Internal {
public:
	static void Init();

	typedef pAnNode (*spawnFunc)();
	struct noderef {
		noderef(std::string s, std::string n, spawnFunc f)
			: sig(s), name(n), spawner(f) {}
		const std::string sig;
		std::string name;
		const spawnFunc spawner;
	};
	static std::array<std::vector<noderef>, (int)ANNODE_GROUP::_COUNT> scrs;
	static std::array<std::string, (int)ANNODE_GROUP::_COUNT> groupNms;
};

class AnNode_Internal_Reg {
	typedef pAnNode (*spawnFunc)();
public:
	AnNode_Internal_Reg(const std::string sig, const std::string nm, ANNODE_GROUP grp, spawnFunc func);
};