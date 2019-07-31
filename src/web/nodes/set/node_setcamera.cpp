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

#include "node_setcamera.h"
#include "vis/pargraphics.h"
#include "md/particles.h"

INODE_DEF(__("Set Camera"), SetCamera, SET)

Node_SetCamera::Node_SetCamera() : INODE_INIT {
    INODE_TITLE(NODE_COL_IO);

    inputR.resize(6);
    auto& vrs = script->invars;
    std::pair<std::string, std::string> pr("center X", "double");
    vrs.resize(6, pr);
    vrs[1].first = "center Y";
    vrs[2].first = "center Z";
    vrs[3].first = "rotation W";
    vrs[4].first = "rotation Y";
    vrs[5].first = "scale";
    inputVDef.resize(6);
    script->invaropts.resize(6);
    OnSceneUpdate();
}

void Node_SetCamera::Execute() {
    OnSceneUpdate();
#define ST(a, nm)\
    if (inputR[a].first) {\
        ParGraphics::nm = (float)getval_d(a);\
        Scene::dirty = true;\
    }
    ST(0, rotCenter.x)
    ST(1, rotCenter.y)
    ST(2, rotCenter.z)
    ST(3, rotZ = ParGraphics::rotZs)
    ST(4, rotW = ParGraphics::rotWs)
    ST(5, rotScale)
#undef ST
}

void Node_SetCamera::OnSceneUpdate() {
    inputVDef[0].d = ParGraphics::rotCenter.x;
    inputVDef[1].d = ParGraphics::rotCenter.y;
    inputVDef[2].d = ParGraphics::rotCenter.z;
    inputVDef[3].d = ParGraphics::rotZ;
    inputVDef[4].d = ParGraphics::rotW;
    inputVDef[5].d = ParGraphics::rotScale;
}

void Node_SetCamera::OnAnimFrame() {
    Execute();
}