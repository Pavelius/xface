#include "collection.h"
#include "crt.h"

extern "C" void* malloc(unsigned size);
extern "C" void* realloc(void *ptr, unsigned size);
extern "C" void	free(void* pointer);

static unsigned rmoptimal(unsigned need_count) {
	const unsigned mc = 256 * 256 * 256;
	unsigned m = 64;
	while(m < mc) {
		if(need_count < m)
			return m;
		m = m << 1;
	}
	return m;
}

amem::~amem() {
	if(data)
		delete data;
	data = 0;
}

void* amem::add() {
	reserve(count + 1);
	auto p = (char*)data + size * count;
	count++;
	return p;
}

void amem::clear() {
	delete data;
	data = 0;
	count = 0;
	count_maximum = 0;
}

void amem::reserve(unsigned count) {
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

void amem::setup(unsigned size) {
	clear();
	this->size = size;
}