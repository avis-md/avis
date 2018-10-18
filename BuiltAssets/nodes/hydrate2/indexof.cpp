#include <iostream>
#include <vector>

//in cnt
double* vals = 0;

//out icnt
int* ids = 0;
std::vector<int> _ids;

//var
int cnt = 0;
//var
int icnt = 0;

//entry
void Do() {
	_ids.clear();
	_ids.reserve(cnt);
	for (int a = 0; a < cnt; a++) {
		if (vals[a] > 0) _ids.push_back(a);
	}
	icnt = _ids.size();
	ids = &_ids[0];
}