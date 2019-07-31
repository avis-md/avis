// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include "Engine.h"

template <class T> std::shared_ptr<T> get_shared(Object* ref) {
	return std::dynamic_pointer_cast<T> (ref->shared_from_this());
}

template <class T> class Ref {
public:
	Ref(bool suppress = false) : _suppress(suppress) {

		static_assert(std::is_base_of<Object, T>::value, "Ref class must be derived from Object!");
	}
	Ref(std::shared_ptr<T> ref, bool suppress = false) : _object(ref), _empty(false), _suppress(suppress) {
		static_assert(std::is_base_of<Object, T>::value, "Ref class must be derived from Object!");
	}

	std::shared_ptr<T> operator()() { //get
		if (_empty) {
			if (!_suppress) Debug::Error("Object Reference", "Reference is null!");
			return nullptr;
		}
		else if (_object.expired()) {
			if (!_suppress) Debug::Error("Object Reference", "Reference is deleted!");
			return nullptr;
		}
		else return _object.lock();
	}
	std::shared_ptr<T> operator->() { return this->operator()(); }

	void operator()(const std::shared_ptr<T>& ref) { //set
		_object = ref;
		_empty = false;
	}
	void operator()(const Ref<T>& ref) {
		_object = ref._object;
		_empty = false;
	}
	void operator()(const T* ref) {
		if (!ref) clear();
		else {
			_object = get_shared<T>((Object*)ref);
			_empty = false;
		}
	}
	operator bool() const {
		return !(_empty || _object.expired());
	}

	bool operator ==(const Ref<T>& rhs) const {
		return this->_object.lock() == rhs._object.lock();
	}
	bool operator !=(const Ref<T>& rhs) const {
		return this->_object.lock() != rhs._object.lock();
	}

	void clear() {
		_empty = true;
	}
	bool ok() {
		return !(_empty || _object.expired());
	}
	T* raw() {
		if (ok()) return _object.lock().get();
		else return nullptr;
	}

private:
	std::weak_ptr<T> _object;
	bool _empty = true, _suppress = false;
};
