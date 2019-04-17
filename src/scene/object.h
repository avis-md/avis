#pragma once

/*! Base class of instantiatable object
[av] */
class Object : public std::enable_shared_from_this<Object> {
public:
	Object(std::string nm = "");
	Object(ulong id, std::string nm);
	ulong id;
	std::string name;
	bool dirty = false; //triggers a reload of internal variables
	bool dead = false; //will be cleaned up after this frame

	virtual bool ReferencingObject(Object* o) { return false; }
};