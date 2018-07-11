#include "makeres.h"

//unsigned char _bg_png[] = "\x12\x11\x22";

void MakeRes::Do() {
	const char* hx = "0123456789ABCDEF";
	char rs[] = "\\x00";
	auto fls = IO::GetFiles(IO::path + "/res/", ".png");
	auto sz = (IO::path + "/res").size();
	std::ofstream strma(IO::path + "/res/src/resdata.h");
	strma << "#pragma once\n";
	for (auto& f : fls) {
		std::cout << "writing " << f << std::endl;
		auto bb = IO::GetBytes(IO::path + "/res/" + f);
		f = f.substr(0, f.size() - 4);
		std::ofstream strm(IO::path + "/res/src/_res/" + f + ".h");
		strm << "namespace res {\nconst unsigned int " << f << "_png_sz = " + std::to_string(bb.size()) + ";\n"
			<< "const unsigned char " << f << "_png[] = \"";
		for (auto& b : bb) {
			rs[2] = hx[b >> 4];
			rs[3] = hx[b & 15];
			strm.write(rs, 4);
		}
		strm << "\";\n}";
		strma << "\n#include \"_res/" << f << ".h\"";
	}
}