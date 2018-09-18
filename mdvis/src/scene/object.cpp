#include "Engine.h"

Object::Object(string nm) : id(0), name(nm) {}
Object::Object(ulong id, string nm) : id(id), name(nm) {}