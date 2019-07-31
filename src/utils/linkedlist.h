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
