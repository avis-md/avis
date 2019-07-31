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

#pragma once
#include "Engine.h"

class ErrorView {
public:
    struct Message {
        std::string name, path;
        int linenum, charnum;
        std::vector<std::string> msg;
        bool severe;
    };

    static std::vector<Message> compileMsgs, execMsgs;

	static bool show, showExec;
    static int descId;
    static int windowSize, descSize;

	static void Refresh();

    static int Parse_GCC(const std::string& path, const std::string& sig, const std::string& name, std::vector<Message>& msgs);
	static int Parse_MSVC(const std::string& path, const std::string& sig, const std::string& name, std::vector<Message>& msgs);
    static int Parse_GFortran(const std::string& path, const std::string& sig, const std::string& name, std::vector<Message>& msgs);
	
    static void Draw();
};