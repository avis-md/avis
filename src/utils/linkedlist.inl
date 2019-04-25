#include "linkedlist.h"

template<typename T>
LinkedNode<T>::LinkedNode() : _prev(-1), _next(~0) {}

template<typename T>
LinkedNode<T>::LinkedNode(const T& v) : value(v), _prev(~0), _next(~0) {}


template<typename T>
LinkedList<T>::LinkedList() : items(3), _first_empty(1) {
	items[0]._next = 0;
}

template<typename T>
LinkedList<T>::LinkedList(const T* data, const size_t sz) {
	_first_empty = sz + 1;
	items.resize(_first_empty);
	for (size_t a = 0; a < sz; a++) {
		items[a + 1].value = data[a];
	}
	items[sz]._next = 0;
}

template<typename T>
void LinkedList<T>::resize(const size_t sz) {
	items.resize(sz + 1);
}

template<typename T>
LinkedNode<T>* LinkedList<T>::first() {
	return &items[_first];
}
template<typename T>
LinkedNode<T>* LinkedList<T>::first_empty() {
	return &items[_first_empty];
}

template<typename T>
LinkedNode<T>* LinkedList<T>::prev(LinkedNode<T>* const n) {
	if (!n->_prev) return nullptr;
	if (n->_prev == ~0) return n-1;
	return &items[n->_prev];
}

template<typename T>
LinkedNode<T>* LinkedList<T>::next(LinkedNode<T>* const n) {
	if (!n->_next) return nullptr;
	if (n->_next == ~0) return n+1;
	return &items[n->_next];
}

template<typename T>
void LinkedList<T>::insert_after(LinkedNode<T>* const p, const LinkedNode<T>& n) {
	auto& i = items[_first_empty];
	auto ne = next(&i);
	i = n;
	auto n2 = next(p);
	p->_next = _first_empty;
	const auto f = items.data();
	i._prev = (uint32_t)(p - f);
	i._next = !n2? 0 : (uint32_t)(n2 - f);
	_first_empty = (uint32_t)(ne - f);
	if (_first_empty == items.size()) {
		resize(_first_empty + 5);
	}
}

template<typename T>
void LinkedList<T>::remove(LinkedNode<T>* n) {
	auto n2 = next(n);
	auto p = prev(n);
	const auto f = items.data();
	p->_next = !n2? 0 : (uint32_t)(n2 - f);
	n2->_prev = (uint32_t)(p - f);
	auto& i = items[_first_empty];
	auto& i2 = items[(i._prev = (n - f))];
	i2._next = _first_empty;
	_first_empty = i._prev;
}