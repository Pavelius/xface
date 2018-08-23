#include "collection.h"
#include "crt.h"

void collection::add(const void* element) {
	auto p = add();
	memcpy(p, element, getsize());
}

int collection::find(const char* value, unsigned offset) {
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

void collection::swap(int i1, int i2) {
	auto p1 = (char*)get(i1);
	auto p2 = (char*)get(i2);
	auto s = getsize();
	for(unsigned i = 0; i < s; i++) {
		char a = p1[i];
		p1[i] = p2[i];
		p2[i] = a;
	}
}

void collection::sort(int i1, int i2, int(*compare)(const void* p1, const void* p2, void* param), void* param) {
	for(int i = i2; i > i1; i--) {
		for(int j = i1; j < i; j++) {
			auto t1 = get(j);
			auto t2 = get(j + 1);
			if(compare(t1, t2, param) > 0)
				swap(j, j + 1);
		}
	}
}