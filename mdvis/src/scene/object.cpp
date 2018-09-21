#include "Engine.h"

Object::Object(std::string nm) : id(0), name(nm) {}
Object::Object(ulong id, std::string nm) : id(id), name(nm) {}
