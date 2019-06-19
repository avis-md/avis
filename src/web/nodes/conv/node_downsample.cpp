#include "node_downsample.h"

#define RETERR(msg) { std::cerr << msg << std::endl; return; }

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
	if (!inputR[0].first) return;
    const auto ratio = getval_i(1);
	if (ratio < 1)
		RETERR("downsample ratio must be a positive integer!");

    std::cout << "Downsampling to ";
    const auto dim = dims.size();
    auto sz = 1;
    std::vector<int> szls(dim);
    for (size_t a = 0; a < dim; a++) {
        const auto dl = inputR[0].getdim(a);
        szls[a] = dl;
        sz *= (dims[a] = (dl + ratio - 1) / ratio + 1);
        std::cout << dims[a] << " ";
    }
    std::cout << std::endl;

    data.clear();
    data.resize(sz);

    assert(dim == 3);

    const float nr = 1.f / std::pow(ratio, 3);
    const int nxl = szls[0];
    const int nyl = szls[1];
    const int nzl = szls[2];
    const int nyzl = nyl * nzl;
    const int nz = dims[2];
    const int nyz = dims[1] * nz;
    const double* input = *(double**)inputR[0].getval(ANVAR_ORDER::C);
    #pragma omp parallel for //collapse(3)
    for (int x = 0; x < nxl; x++) {
        for (int y = 0; y < nyl; y++) {
            for (int z = 0; z < nzl; z++) {
                int ix = (x + ratio - 1) / ratio;
                int iy = (y + ratio - 1) / ratio;
                int iz = (z + ratio - 1) / ratio;
                data[ix * nyz + iy * nz + iz] += input[x * nyzl + y * nzl + z] * nr;
            }
        }
    }

	auto& ov = ((DmScript_I*)script.get())->outputVs;
	ov[0].val.p = data.data();
	ov[0].pval = &ov[0].val.p;
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