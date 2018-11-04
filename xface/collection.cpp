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

int array::find(const char* value, unsigned offset) const {
	auto m = getcount();
	for(unsigned i = 0; i < m; i++) {
		auto p = (const char**)((char*)get(i) + offset);
		if(!(*p))
			continue;
		if(strcmp(*p, value) == 0)
			return i;
	}
	return -1;
}

void array::swap(int i1, int i2) {
	auto p1 = (char*)get(i1);
	auto p2 = (char*)get(i2);
	auto s = getsize();
	for(unsigned i = 0; i < s; i++) {
		char a = p1[i];
		p1[i] = p2[i];
		p2[i] = a;
	}
}

void array::sort(int i1, int i2, int(*compare)(const void* p1, const void* p2, void* param), void* param) {
	for(int i = i2; i > i1; i--) {
		for(int j = i1; j < i; j++) {
			auto t1 = get(j);
			auto t2 = get(j + 1);
			if(compare(t1, t2, param) > 0)
				swap(j, j + 1);
		}
	}
}

void array::remove(int index, int elements_count) {
	if(((unsigned)index) >= count)
		return;
	if((unsigned)index < count - elements_count)
		memcpy(get(index), get(index + elements_count), (count - (index + elements_count))*getsize());
	count -= elements_count;
}

int	array::indexof(const void* element) const {
	if(element >= data && element < ((char*)data + size*count))
		return ((char*)element - (char*)data) / size;
	return -1;
}

void* array::insert(int index, const void* element) {
	auto count_before = getcount(); add();
	memmove((char*)data + (index + 1)*size, (char*)data + index * size, (count_before - index)*size);
	void* p = get(index);
	if(element)
		memcpy(p, element, size);
	else
		memset(p, 0, size);
	return p;
}

void array::add(const void* element) {
	auto p = add();
	memcpy(p, element, getsize());
}