#include "main.h"

void* database::get(int index) const {
	auto p = elements;
	if(!p)
		return 0;
	while((unsigned)index > p->count) {
		index -= p->count;
		p = p->next;
		if(!p)
			return 0;
	}
	return (char*)p + sizeof(arrayseq) + size * index;
}

void* database::add() {
	auto p = elements;
	if(p) {
		p = p->last();
		if(p->count < p->count_maximum)
			return (char*)p + sizeof(arrayseq) + (p->count++)*size;
	}
	auto count_maximum = 0;
	if(p)
		count_maximum = p->count_maximum * 2;
	if(!count_maximum)
		count_maximum = 64;
	if(count_maximum > 256 * 256)
		count_maximum = 256 * 256;
	if(!p) {
		p = (arrayseq*)(new char[sizeof(arrayseq) + count_maximum * size]);
		elements = p;
	}
	else {
		p->next = (arrayseq*)(new char[sizeof(arrayseq) + count_maximum*size]);
		p = p->next;
	}
	p->count_maximum = count_maximum;
	p->count = 1;
	p->next = 0;
	return (char*)p + sizeof(arrayseq);
}

//int database::indexof(const void* object) const {
//	auto base_index = 0;
//	for(const datastore* p = this; p; p = p->next) {
//		if(!p->data)
//			break;
//		if(object >= (char*)p->data && object < (char*)p->data + p->count*size)
//			return base_index + ((char*)object - (char*)p->data) / size;
//		base_index += p->count;
//	}
//	return -1;
//}

database::~database() {
	auto p = elements;
	while(p) {
		auto m = p->next;
		delete p;
		p = m;
	}
	elements = 0;
}