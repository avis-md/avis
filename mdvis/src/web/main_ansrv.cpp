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
            AnBrowse::PreC(forceCompile);
            break;
        case 1: //fortran
            break;
        }
    }
    else {
        std::ofstream strm(IO::path + "/.lock");
        if (!strm.is_open()) return -1;
        strm << "hoge";
        strm.close();

        std::cout << "Initializing python..." << std::endl;

        PyReader::Init();

        std::cout << "Parsing nodes..." << std::endl;

        AnBrowse::Scan();

        std::cout << "Reproducing nodes..." << std::endl;
        
        AnWeb::Load(IO::path + "/ser/web.anl");

        for (auto& n : AnWeb::nodes)
            std::cout << "\"" + n->title + "\"" << " ";
        std::cout << std::endl;

        AnWeb::LoadIn();

        std::cout << "Executing... ";
        std::flush(std::cout);

        AnWeb::DoExecute();

        AnWeb::SaveOut();

        int sz = *AnWeb::nodes[2]->conV[0].dimVals[0];
        std::cout << "size is: " << sz << std::endl;
        float* dt = *((float**)AnWeb::nodes[2]->conV[0].value);
        for (int i = 0; i < sz; i++) {
            std::cout << dt[i] << "\n";
        }

        std::cout << "finished" << std::endl;

        remove((IO::path + "/.lock").c_str());
    }
}