#include "node_camera.h"
#include "vis/pargraphics.h"
#include "md/particles.h"

Node_Camera_Out::Node_Camera_Out() : AnNode(new DmScript(sig)) {
    title = "Camera (Set)";
    titleCol = NODE_COL_IO;
    canTile = false;
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

void Node_Camera_Out::Execute() {
    OnSceneUpdate();
#define ST(a, nm)\
    if (inputR[a].first) {\
        ParGraphics::nm = (float)(*(double*)inputR[a].first->conV[inputR[a].second].value);\
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

void Node_Camera_Out::OnSceneUpdate() {
    inputVDef[0].d = ParGraphics::rotCenter.x;
    inputVDef[1].d = ParGraphics::rotCenter.y;
    inputVDef[2].d = ParGraphics::rotCenter.z;
    inputVDef[3].d = ParGraphics::rotZ;
    inputVDef[4].d = ParGraphics::rotW;
    inputVDef[5].d = ParGraphics::rotScale;
}

void Node_Camera_Out::OnAnimFrame() {
    Execute();
}