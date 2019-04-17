#include "node_.h"
#include "web/anweb.h"

INODE_DEF(__(""), , MISC)

Node_::Node_() : INODE_INIT {
	INODE_TITLE(NODE_COL_NRM);
	INODE_SINIT(
		scr->desc = R"()";
		scr->descLines = 0;
		
		scr->AddInput(_(""), AN_VARTYPE::INT, 1);
		scr->AddOutput(_(""), AN_VARTYPE::INT, 1);
	);

	IAddConV(0, {  }, {  });
}