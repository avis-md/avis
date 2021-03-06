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

#include "makeres.h"

//unsigned char _bg_png[] = "\x12\x11\x22";

void MakeRes::Do() {
	const char* hx = "0123456789ABCDEF";
	auto fls = IO::GetFiles(IO::path + "res/", "*.png");
	auto sz = (IO::path + "res").size();
	std::ofstream strma(IO::path + "res/src/resdata.h", std::ios::app);
	//strma << "#pragma once\n";
	for (auto& f : fls) {
		std::cout << "writing " << f << std::endl;
		auto bb = IO::GetBytes(IO::path + "res/" + f);
		f = f.substr(0, f.size() - 4);
		std::ofstream strm(IO::path + "res/src/_res/" + f + ".h");
		strm << "namespace res {\nconst unsigned int " << f << "_png_sz = " + std::to_string(bb.size()) + ";\n"
			<< "const unsigned char " << f << "_png[] = {";
		for (size_t a = 0, s = bb.size(); a < s; ++a) {
			strm << std::to_string(bb[a]);
			if (a < s-1) strm << ",";
		}
		strm << "};\n}";
		strma << "\n#include \"_res/" << f << ".h\"";
	}
}