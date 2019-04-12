#include "main.h"

database::element* database::element::last() {
	for(auto p = this; p; p = p->next) {
		if(!p->next)
			return p;
	}
	return 0;
}

void* database::get(int index) const {
	auto p = &first;
	while((unsigned)index > p->count) {
		index -= p->count;
		p = p->next;
		if(!p)
			return 0;
	}
	if(!p->data)
		return 0;
	return (char*)p->data + size * index;
}

void* database::add() {
	auto p = first.last();
	if(p->count < p->count_maximum)
		return (char*)p->data + (p->count++)*size;
	auto count_maximum = p->count_maximum*2;
	if(!count_maximum)
		count_maximum = 256;
	if(count_maximum > 256 * 256)
		count_maximum = 256 * 256;
	if(p->data) {
		p->next = new element;
		p = p->next;
	}
	p->count_maximum = count_maximum;
	p->data = new char[count_maximum * size];
	p->count = 1;
	return p->data;
}

unsigned database::getcount() const {
	auto result = first.count;
	for(auto p = first.next; p; p = p->next)
		result += p->count;
	return result;
}

unsigned database::getmaxcount() const {
	auto result = first.count_maximum;
	for(auto p = first.next; p; p = p->next)
		result += p->count_maximum;
	return result;
}

int database::indexof(const void* object) const {
	auto base_index = 0;
	for(auto p = &first; p; p = p->next) {
		if(!p->data)
			break;
		if(object >= (char*)p->data && object < (char*)p->data + p->count*size)
			return base_index + ((char*)object - (char*)p->data) / size;
		base_index += p->count;
	}
	return -1;
}

database::~database() {
	auto p = &first;
	while(p) {
		auto m = p->next;
		if(m)
			delete m;
		p = m;
	}
}