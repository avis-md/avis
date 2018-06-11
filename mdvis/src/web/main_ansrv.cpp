#include "Engine.h"
#include "anweb.h"

int main(int argc, char** argv) {
    PyReader::Init();
	AnNode::Init();

	AnBrowse::Scan();
	
	AnWeb::Init();

	AnWeb::Load(IO::path + "/nodes/rdf.anl");
	AnWeb::LoadIn();
	AnWeb::Execute();

    int sz = *AnWeb::nodes[2]->conV[0].dimVals[0];
    std::cout << "size is: " << sz << std::endl;
    float* dt = (float*)AnWeb::nodes[2]->conV[0].value;
    for (int i = 0; i < sz; i++) {
        std::cout << dt[i] << "\n";
    }
    std::flush(std::cout);
}