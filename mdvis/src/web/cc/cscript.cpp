#include "web/anweb.h"

std::shared_ptr<AnScript_I> CScript::CreateInstance() {
	auto res = std::make_shared<CScript_I>();
	res->parent = this;
	res->instance = spawner();
	return res;
}

void CScript_I::SetInput(int i, int val) {

}
void CScript_I::GetOutput(int i, int* val) {

}