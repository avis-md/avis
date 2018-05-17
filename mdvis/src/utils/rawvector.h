/*
 * ?
 */

#pragma once
#include <cstdlib>
#include <new>

template <typename T, typename S> class rawvector {
public:
	typedef T* type;

	//rawvector() : _pointer(dummyVal), size(dummySz) {}

	rawvector(T*& pointer, S& size, bool alloc = true) : _pointer(&pointer), size(&size) {
		if (alloc) {
			size = 0;
			*_pointer = (T*)malloc(sizeof(T));
			//T val = T();
			//memcpy(*_pointer, &val, sizeof(T));
		}
	}

	rawvector<T, S>& operator= (const rawvector<T, S>& rhs) {
		this->_pointer = rhs._pointer;
		this->size = rhs.size;
		return *this;
	}

	void push(const T& val) {
		if (!(*size)) {
			memcpy(&(*_pointer)[(*size)++], &val, sizeof(T));
		}
		else {
			auto res = (T*)realloc(*_pointer, sizeof(T)*(*size + 1));
			if (!res) throw std::bad_alloc();
			else {
				*_pointer = res;
				memcpy(&(*_pointer)[(*size)++], &val, sizeof(T));
			}
		}
	}
private:
	T** _pointer;
	S* size;
};

void rawvector_free(void* v) {
	free(v);
}