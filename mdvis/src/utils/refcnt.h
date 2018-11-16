#pragma once
#include "Engine.h"

template <typename T>
class RefCnt {
protected:
	RefCnt() {
		_refkey = _refcounter++;
		++_refs[_refkey];
	}
	RefCnt(RefCnt const& rhs) : _refkey(rhs._refkey) {
		++_refs[_refkey];
	}
	~RefCnt() {
		--_refs[_refkey];
	}

	RefCnt& operator= (RefCnt const& rhs) {
		--_refs[_refkey];
		_refkey = rhs._refkey;
		++_refs[_refkey];
		return *this;
	}

	bool _IsSingleRef() {
		const auto cnt = _refs[_refkey];
		return (cnt == 1);
	}

private:
	uint _refkey;
	static uint _refcounter;
	static std::unordered_map<uint, uint> _refs;
};

template <typename T>
uint RefCnt<T>::_refcounter = 0;

template <typename T>
std::unordered_map<uint, uint> RefCnt<T>::_refs = {};