#include "localizer.h"

void Localizer::Init(const string& nm) {
    auto path = IO::path + "/locale/" + nm + "/locale.txt";
    std::ifstream strm(path);
    if (!strm.is_open()) {
        Debug::Warning("System", "locale not found! defaulting to english...");
        path = IO::path + "/locale/EN/locale.txt";
        strm.open(path);
        if (!strm.is_open()) {
            Debug::Error("System", "EN locale not found!");
            abort();
        }
    }
    char buf[500]{};
    dict.clear();
    std::pair<uint32_t, string> pr;
    int l = 1;
    while (strm.getline(buf, 20)) {
        if (!buf[0]) {
            l++;
            continue;
        }
        pr.first = TryParse(string(buf), -1U);
        if (pr.first == -1U) {
            Debug::Error("System", "Locale syntax error at line " + std::to_string(l));
            abort();
        }
        strm.getline(buf, 500);
        if (buf[0] != '<') {
            Debug::Error("System", "Locale syntax error at line " + std::to_string(l + 1));
            abort();
        }
        strm.getline(buf, 500);
        if (buf[0] != '>') {
            Debug::Error("System", "Locale syntax error at line " + std::to_string(l + 2));
            abort();
        }
        pr.second = buf + 1;
        dict.emplace(pr);
        l += 3;
    }
}

void Localizer::MakeMap(string path) {
    path = IO::path + "/" + path;
    std::map<uint32_t, string> strs;
    _MakeMap(path, strs);
    std::ofstream strm(IO::path + "/locale/template.txt");
    std::ofstream strm2(IO::path + "/locale/EN/locale.txt");
    for (auto& a : strs) {
        strm << a.first << "\n<" << a.second << "\n>\n\n";
        strm2 << a.first << "\n<" << a.second << "\n>" + a.second + "\n\n";
    }
}

void Localizer::_MakeMap(string path, std::map<uint32_t, string>& strs) {
    std::vector<string> nms = IO::GetFiles(path, ".cpp");
    for (auto& n : nms) {
        string cd = IO::GetText(path + "/" + n);
        auto cds = cd.size();
        int p0 = 0, p1 = 0;
        for(;;) {
            p0 = string_find(cd, "_(\"", p1);
            if (p0 == -1) break;
            p1 = string_find(cd, "\")", p0);
            auto str = cd.substr(p0 + 3, p1 - p0 - 3);
            strs[HASH(str.c_str())] = str;
        }
    }
    nms.clear();
    IO::GetFolders(path, &nms);
    for (auto& n : nms)
        _MakeMap(path + "/" + n, strs);
}

std::unordered_map<uint32_t, string> Localizer::dict;