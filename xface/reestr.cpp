#include "reestr.h"

unsigned stridx::find(const void* p, unsigned c) {
	auto pb = (char*)begin();
	auto pe = (char*)end() - c;
	while(pb < pe) {
		if(memcmp(pb, p, c) == 0)
			return pb - begin();
		pb++;
	}
	return 0xFFFFFFFF;
}

unsigned stridx::add(const char* id) {
	auto c = zlen(id) + 1;
	auto i = find(id, c);
	if(i != 0xFFFFFFFF)
		return i;
	i = count;
	reserve(i + c);
	memcpy(ptr(i), id, c);
	return i;
}