#include "collection.h"
#include "crt.h"

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