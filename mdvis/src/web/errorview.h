#pragma once
#include "Engine.h"

class ErrorView {
public:
    struct Message {
        string name, path;
        int linenum, charnum;
        std::vector<string> msg;
        bool severe;
    };

    static std::vector<Message> compileMsgs, execMsgs;

	static bool show, showExec;
    static int descId;
    static int windowSize, descSize;

	static void Refresh();

    static int Parse_GCC(const string& path, const string& sig, const string& name, std::vector<Message>& msgs);
	static int Parse_MSVC(const string& path, const string& sig, const string& name, std::vector<Message>& msgs);

    static void Draw();
};