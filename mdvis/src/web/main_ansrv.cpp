#include "Engine.h"
#include "anweb.h"

int compileType = -1;
bool forceCompile = false;

int main(int argc, char** argv) {
    for (int a = 0; a < argc; a++) {
        if (!strcmp(argv[a], "-f")) {
            forceCompile = true;
        }
        else if (!strcmp(argv[a], "-c")) {
            compileType = argv[a+1][0] - '1';
        }
    }

    if (compileType > -1) {
        std::vector<string> ff;
        switch (compileType) {
        case 0: //c++
		    std::cout << "Compiling c++ nodes..." << std::endl;
            AnBrowse::PreC();
            break;
        case 1: //fortran
            break;
        }
    }
    else {
        std::ofstream strm(IO::path + "/.lock");
        strm << "hoge";
        strm.close();

        PyReader::Init();
        AnNode::Init();

        AnBrowse::Scan();
        
        AnWeb::Init();

        AnWeb::Load(IO::path + "/ser/web.anl");
        AnWeb::LoadIn();
        AnWeb::Execute();

        int sz = *AnWeb::nodes[2]->conV[0].dimVals[0];
        std::cout << "size is: " << sz << std::endl;
        float* dt = (float*)AnWeb::nodes[2]->conV[0].value;
        for (int i = 0; i < sz; i++) {
            std::cout << dt[i] << "\n";
        }
        std::flush(std::cout);

        remove((IO::path + "/.lock").c_str());
    }
}