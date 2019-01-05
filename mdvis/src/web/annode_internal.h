#pragma once
#include "web/annode.h"
#include "web/anscript.h"
#include "ui/localizer.h"

#define INODE_DEF_H(nm) \
	static const std::string sig;\
	static const char* _name;\
	static DmScript scr;\
	static AnNode_Internal_Reg _reg;\
	static AnNode* _Spawn() { return new Node_ ## nm(); } 

#define INODE_DEF(tt, nm, gp) \
	const std::string Node_ ## nm::sig = "." ## #nm;\
	const char* Node_ ## nm::_name = tt;\
	DmScript Node_ ## nm::scr = DmScript(Node_ ## nm::sig);\
	AnNode_Internal_Reg Node_ ## nm::_reg = AnNode_Internal_Reg(\
		Node_ ## nm::sig, Node_ ## nm::_name, ANNODE_GROUP::gp, &Node_ ## nm::_Spawn);

#define INODE_TITLE(col) \
	title = _(_name);\
	titleCol = col;

enum class AN_NODE_SCN : byte {
	NUM0 = 0x80,
	OCAM,
	NUM
};
const std::string AN_NODE_SCNS[] = { "Camera (Set)" };

enum class AN_NODE_IN : byte {
	NUM0 = 0,
	PARS,
	PINFO,
	VEC,
	NUM
};
const std::string AN_NODE_INS[] = { "Particle data", "System info", "Vector" };

enum class AN_NODE_MOD {
	NUM0 = 0x20,
	GATTR,
	SATTR,
	RSCL,
	SBXC,
	NUM
};
const std::string AN_NODE_MODS[] = { "Get Param", "Set Param", "Set Radii Scale", "Set BBox Center" };

enum class AN_NODE_GEN {
	NUM0 = 0x40,
	BOND,
	SURF,
	TRJ,
	NUM
};
const std::string AN_NODE_GENS[] = { "Add Bonds", "Draw Surface", "Trace Trajectory" };

enum class AN_NODE_MISC {
	NUM0 = 0x60,
	PLOT,
	SRNG,
	ADJL,
	ADJLI,
	DOWHL,
	NUM
};
const std::string AN_NODE_MISCS[] = { "Plot graph", "Show Range", "To Adjacency List", "To Paired List", "Do While" };

enum class ANNODE_GROUP {
	USER,
	GET,
	SET,
	GEN,
	MISC
};

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
	static std::array<std::vector<noderef>, 4> scrs;
	static std::array<std::string, 4> groupNms;
};

class AnNode_Internal_Reg {
public:
	AnNode_Internal_Reg(const std::string sig, const std::string nm, ANNODE_GROUP grp, AnNode* (*func)());
};


#include "nodes/node_showrange.h"
#include "nodes/node_setradscl.h"
#include "nodes/node_tracetrj.h"
#include "nodes/node_volume.h"
#include "nodes/node_remap.h"
#include "nodes/node_gromacs.h"
#include "nodes/node_inputs.h"
#include "nodes/node_info.h"
#include "nodes/node_addbond.h"
#include "nodes/node_camera.h"

#include "nodes/in/node_vector.h"

#include "nodes/get/node_getattribute.h"

#include "nodes/set/node_setattribute.h"
#include "nodes/set/node_setbboxcenter.h"

#include "nodes/mod/node_addsurface.h"

#include "nodes/misc/node_plot.h"
#include "nodes/misc/node_adjlist.h"
#include "nodes/misc/node_accum.h"

#include "nodes/ctrl/node_dowhile.h"