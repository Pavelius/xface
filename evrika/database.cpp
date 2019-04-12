#include "main.h"

datastore* datastore::last() {
	for(auto p = this; p; p = p->next) {
		if(!p->next)
			return p;
	}
	return 0;
}

unsigned datastore::getcount() const {
	auto result = count;
	for(auto p = next; p; p = p->next)
		result += p->count;
	return result;
}

unsigned datastore::getmaxcount() const {
	auto result = count_maximum;
	for(auto p = next; p; p = p->next)
		result += p->count_maximum;
	return result;
}

void* database::get(int index) const {
	auto p = (datastore*)this;
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
	auto p = last();
	if(p->count < p->count_maximum)
		return (char*)p->data + (p->count++)*size;
	auto count_maximum = p->count_maximum*2;
	if(!count_maximum)
		count_maximum = 64;
	if(count_maximum > 256 * 256)
		count_maximum = 256 * 256;
	if(p->data) {
		p->next = new datastore;
		p = p->next;
	}
	p->count_maximum = count_maximum;
	p->data = new char[count_maximum * size];
	p->count = 1;
	return p->data;
}

int database::indexof(const void* object) const {
	auto base_index = 0;
	for(const datastore* p = this; p; p = p->next) {
		if(!p->data)
			break;
		if(object >= (char*)p->data && object < (char*)p->data + p->count*size)
			return base_index + ((char*)object - (char*)p->data) / size;
		base_index += p->count;
	}
	return -1;
}

database::~database() {
	auto p = next;
	while(p) {
		auto m = p->next;
		if(p)
			delete p;
		p = m;
	}
}