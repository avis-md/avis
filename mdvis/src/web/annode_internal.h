#pragma once
#include "web/annode.h"
#include "web/anscript.h"
#include "ui/localizer.h"

#define INODE_DEF_H \
	static const std::string sig;\
	static const char* _name;\
	DmScript scr;\
	static AnNode_Internal_Reg _reg;\
	static AnNode* _Spawn();

#define INODE_DEF(tt, nm, gp) \
	const std::string Node_ ## nm::sig = "." ## #nm;\
	const char* Node_ ## nm::_name = tt;\
	AnNode_Internal_Reg Node_ ## nm::_reg = AnNode_Internal_Reg(\
		Node_ ## nm::sig, Node_ ## nm::_name, ANNODE_GROUP::gp, &Node_ ## nm::_Spawn);\
	AnNode* Node_ ## nm::_Spawn() { return new Node_ ## nm(); } 

#define INODE_INIT AnNode(&scr), scr(sig)
#define INODE_INITF(f) AnNode(&scr, f), scr(sig)

#define INODE_TITLE(col) \
	title = _(_name);\
	titleCol = col;

enum class ANNODE_GROUP {
	GET,
	SET,
	GEN,
	CONV,
	MISC
};
#define ANNODE_GROUP_COUNT 5

class AnNode_Internal {
public:
	static void Init();

	typedef AnNode* (*spawnFunc)();
	struct noderef {
		noderef(std::string s, std::string n, spawnFunc f)
			: sig(s), name(n), spawner(f) {}
		const std::string sig;
		std::string name;
		const spawnFunc spawner;
	};
	static std::array<std::vector<noderef>, ANNODE_GROUP_COUNT> scrs;
	static std::array<std::string, ANNODE_GROUP_COUNT> groupNms;
};

class AnNode_Internal_Reg {
public:
	AnNode_Internal_Reg(const std::string sig, const std::string nm, ANNODE_GROUP grp, AnNode* (*func)());
};

#include "nodes/get/node_inputs.h"
#include "nodes/get/node_info.h"
#include "nodes/get/node_getattribute.h"

#include "nodes/set/node_setattribute.h"
#include "nodes/set/node_setbboxcenter.h"
#include "nodes/set/node_showrange.h"
#include "nodes/set/node_setradscl.h"

#include "nodes/in/node_vector.h"

#include "nodes/mod/node_addbond.h"
#include "nodes/mod/node_addsurface.h"

#include "nodes/conv/node_adjlist.h"
#include "nodes/conv/node_remap.h"

#include "nodes/misc/node_plot.h"
#include "nodes/misc/node_accum.h"

#include "nodes/ctrl/node_dowhile.h"

#include "nodes/node_tracetrj.h"
#include "nodes/node_volume.h"
#include "nodes/node_camera.h"