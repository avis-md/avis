#pragma once
#include "Engine.h"

class RefCnt {
protected:
	RefCnt() {
		_ptr = std::make_shared<char>(' ');
	}
	RefCnt(RefCnt const& rhs) : _ptr(rhs._ptr) {}
	RefCnt& operator= (RefCnt const& rhs) {
		CheckUniqueRef();
		_ptr = rhs._ptr;
		return *this;
	}
	void CheckUniqueRef() {
		if (_ptr.unique()) DestroyRef();
	}
	virtual void DestroyRef() = 0;
private:
	std::shared_ptr<char> _ptr;
};