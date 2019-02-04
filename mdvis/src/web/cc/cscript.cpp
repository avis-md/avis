#include "web/anweb.h"

std::unordered_map<std::string, std::weak_ptr<CScript>> CScript::allScrs;

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