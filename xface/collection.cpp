#include "collection.h"
#include "crt.h"

extern "C" void* malloc(unsigned size);
extern "C" void* realloc(void *ptr, unsigned size);
extern "C" void	free(void* pointer);

unsigned rmoptimal(unsigned need_count) {
	const unsigned mc = 256 * 256 * 256;
	unsigned m = 16;
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

void* arraydata::add(unsigned size) {
	auto p = this;
	while(p->count >= p->maximum) {
		if(!p->next) {
			auto m = p->maximum * 2;
			auto s = sizeof(*this) + size * m;
			p->next = (arraydata*)new char[s];
			p->next->count = 0;
			p->next->next = 0;
			p->next->maximum = m;
		}
		p = p->next;
	}
	return (char*)p + sizeof(arraydata) + (p->count++)*size;
}

void arraydata::release() {
	auto p = next;
	next = 0;
	count = 0;
	while(p) {
		auto n = p->next;
		delete p;
		p = n;
	}
}

unsigned arraydata::getcount() const {
	unsigned result = 0;
	for(auto p = this; p; p = p->next)
		result += p->count;
	return result;
}

void* arraydata::get(int index, unsigned size) const {
	for(auto p = this; p; p = p->next) {
		if((unsigned)index < p->count)
			return (char*)p + sizeof(*this) + index * size;
		index -= p->count;
	}
	return 0;
}

void arraydata::clear() {
	for(auto p = this; p; p = p->next)
		p->count = 0;
}

int arraydata::indexof(const void* e, unsigned size) const {
	for(auto p = this; p; p = p->next) {
		if(e >= (char*)p + sizeof(*this)
			&& e <= (char*)p + sizeof(*this) + size * maximum)
			return (((char*)e - (char*)p) - sizeof(*this)) / size;
	}
	return -1;
}

void arraydata::iterator::initialize(const arraydata* source, unsigned size) {
	this->source = source;
	if(!source)
		pb = pe = 0;
	else {
		pb = (char*)source->begin();
		pe = pb + source->count*size;
	}
}

void arraydata::iterator::increment(unsigned size) {
	if(pb >= pe)
		initialize(source->next, size);
	else
		pb += size;
}

void* array::add() {
	if(count >= count_maximum) {
		if(can_grow)
			reserve(count + 1);
		else
			return (char*)data;
	}
	return (char*)data + size * (count++);
}

void* array::add(const void* element) {
	auto p = add();
	memcpy(p, element, getsize());
	return p;
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

void array::swap(int i1, int i2) {	
	unsigned char* a1 = (unsigned char*)get(i1);
	unsigned char* a2 = (unsigned char*)get(i2);
	for(unsigned i = 0; i < size; i++) {
		char a = a1[i];
		a1[i] = a2[i];
		a2[i] = a;
	}
}

void array::shift(int i1, int i2, unsigned c1, unsigned c2) {
	if(i2 < i1) {
		iswap(i2, i1);
		iswap(c1, c2);
	}
	unsigned char* a1 = (unsigned char*)get(i1);
	unsigned char* a2 = (unsigned char*)get(i2);
	unsigned s1 = c1 * size;
	unsigned s2 = c2 * size;
	unsigned s = (a2 - a1) + s2 - 1;
	for(unsigned i = 0; i < s1; i++) {
		auto a = a1[0];
		memcpy(a1, a1 + 1, s);
		a1[s] = a;
	}
}