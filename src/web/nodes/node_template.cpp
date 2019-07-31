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