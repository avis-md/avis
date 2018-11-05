#include "node_getcenter.h"

Node_GetCenter::Node_GetCenter() : AnNode(new DmScript(sig)) {
	title = "Get Center";
	titleCol = NODE_COL_NRM;
	inputR.resize(1);
	inputVDef.resize(1);
	script->invaropts.resize(1);
	script->invars.push_back(std::pair<std::string, std::string>("values", "list(2d)"));

	outputR.resize(1);
	script->outvars.resize(1, std::pair<std::string, std::string>("result", "list(1d)"));
}

void Node_GetCenter::Execute() {
    if (!inputR[0].first) return;
	double* poss = *(double**)inputR[0].getval();
	auto& cv = inputR[0].getconv();
	auto sz = *cv.dimVals[0];
	auto dm = *cv.dimVals[1];
	std::vector<double> centers(dm);
	if (periodic) { //use fft
		//(https://stackoverflow.com/questions/18166507/using-fft-to-find-the-center-of-mass-under-periodic-boundary-conditions#23494334)
		
	}
	else { //just get the average
		for (int a = 0; a < dm; ++a) {

		}
	}
}

void Node_GetCenter::LoadOut(const std::string& path) {
    Execute();
}