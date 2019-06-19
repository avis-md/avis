#include "node_downsample.h"

INODE_DEF(__("Downsample"), Downsample, CONV);

Node_Downsample::Node_Downsample() : INODE_INIT {
    INODE_TITLE(NODE_COL_NRM)
	INODE_SINIT(
		scr->AddInput(_("data"), AN_VARTYPE::DOUBLE, -1);
		scr->AddInput(_("ratio"), AN_VARTYPE::INT);

		scr->AddOutput(_("result"), AN_VARTYPE::DOUBLE, 1);
	);

    IAddConV(0, {}, { 0 });
}

void Node_Downsample::Execute() {
    return;
}

void Node_Downsample::OnConn(bool o, int i) {
    if (!o && (i == 0)) {
        const auto dimo = scr->outputs[0].dim;
        IClearConV();
        const auto dim = inputR[0].getconv().szOffsets.size();
        dims.resize(dim);
        std::vector<int*> pdims;
        for (auto& d : dims) {
            pdims.push_back(&d);
        }
        IAddConV(0, pdims);
        scr->outputs[0].dim = dim;
        scr->outputs[0].InitName();

        if (dim != dimo) {
            Disconnect(0, true);
        }
    }
}