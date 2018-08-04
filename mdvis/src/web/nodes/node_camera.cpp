#include "node_camera.h"
#include "vis/pargraphics.h"
#include "md/Particles.h"

Node_Camera_Out::Node_Camera_Out() : AnNode(new DmScript(sig)) {
    title = "Camera (Set)";
    titleCol = Vec3(0.3f, 0.5f, 0.3f);
    canTile = false;
    inputR.resize(6);
    auto& vrs = script->invars;
    std::pair<string, string> pr("", "float");
    vrs.resize(6, pr);
    vrs[0].first = "center X";
    vrs[1].first = "center Y";
    vrs[2].first = "center Z";
    vrs[3].first = "rotation W";
    vrs[4].first = "rotation Y";
    vrs[5].first = "scale";
    inputVDef.resize(6);
    OnSceneUpdate();
}

void Node_Camera_Out::Execute() {
#define ST(a, nm) f = ParGraphics::nm;\
if (inputR[a].first) {\
    ParGraphics::nm = *(float*)inputR[a].first->conV[inputR[a].second].value;\
} else {\
    ParGraphics::nm = inputVDef[a].f;\
}\
if (f != ParGraphics::nm) Scene::dirty = true;
    float f;
    ST(0, rotCenter.x)
    ST(1, rotCenter.y)
    ST(2, rotCenter.z)
    ST(3, rotZs)
    ST(4, rotWs)
    ST(5, rotScale)
}

void Node_Camera_Out::OnSceneUpdate() {
    inputVDef[0].f = ParGraphics::rotCenter.x;
    inputVDef[1].f = ParGraphics::rotCenter.y;
    inputVDef[2].f = ParGraphics::rotCenter.z;
    inputVDef[3].f = ParGraphics::rotZs;
    inputVDef[4].f = ParGraphics::rotWs;
    inputVDef[5].f = ParGraphics::rotScale;
}