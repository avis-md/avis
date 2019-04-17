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