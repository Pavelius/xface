#include "collection.h"
#include "crt.h"

extern "C" void* malloc(unsigned size);
extern "C" void* realloc(void *ptr, unsigned size);
extern "C" void	free(void* pointer);

unsigned rmoptimal(unsigned need_count) {
	const unsigned mc = 256 * 256 * 256;
	unsigned m = 64;
	while(m < mc) {
		if(need_count < m)
			return m;
		m = m << 1;
	}
	return m;
}

void* rmreserve(void* data, unsigned new_size) {
	if(data)
		return realloc(data, new_size);
	return malloc(new_size);
}

array::~array() {
	if(can_grow)
		clear();
}

void array::clear() {
	count = 0;
	if(!can_grow)
		return;
	count_maximum = 0;
	if(data)
		delete data;
	data = 0;
}

void array::setup(unsigned size) {
	if(!can_grow)
		return;
	clear();
	this->size = size;
}

void* array::add() {
	if(can_grow) {
		reserve(count + 1);
		auto p = (char*)data + size * count;
		count++;
		return p;
	}
	return (char*)data + getsize()*((count < count_maximum) ? count++ : 0);
}

void array::reserve(unsigned count) {
	if(!can_grow)
		return;
	if(!size)
		return;
	if(data && count < count_maximum)
		return;
	count_maximum = rmoptimal(count);
	if(data)
		data = realloc(data, count_maximum*size);
	else
		data = malloc(count_maximum*size);
}