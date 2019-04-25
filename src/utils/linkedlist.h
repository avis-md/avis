#pragma once
#include "Engine.h"

template<typename T>
struct LinkedNode {
	LinkedNode();
	LinkedNode(const T&);

	T value = T();

	//int8_t _flags;
	uint32_t _prev;
	uint32_t _next;
};

template<typename T>
class LinkedList {
public:
	LinkedList();
	LinkedList(const T* data, const size_t sz);

	std::vector<LinkedNode<T>> items;

	void resize(const size_t sz);

	LinkedNode<T>* first();
	LinkedNode<T>* first_empty();

	LinkedNode<T>* prev(LinkedNode<T>* const);
	LinkedNode<T>* next(LinkedNode<T>* const);

	void insert_after(LinkedNode<T>* const p, const LinkedNode<T>& n);
	void remove(LinkedNode<T>* n);

//private:
	size_t _first;
	size_t _last;
	size_t _first_empty;
};

#include "linkedlist.inl"
