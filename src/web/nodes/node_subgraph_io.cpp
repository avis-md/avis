#include "node_subgraph_io.h"

INODE_DEF_NOREG(__("Inputs"), Subgraph_In);

Node_Subgraph_In::Node_Subgraph_In() : INODE_INIT {
	INODE_TITLE(NODE_COL_SPC)
	INODE_SINIT(
		scr->AddOutput(_("Add..."), AN_VARTYPE::ANY);
	);

	IAddConV(0, {}, {});
}