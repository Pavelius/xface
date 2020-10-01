#include "archive.h"
#include "crt.h"
#include "strlib.h"

unsigned strlib::add(const char* v) {
	if(!v || v[0] == 0)
		return 0xFFFFFFFF;
	for(unsigned i = 0; i < indecies.count; i++) {
		if(strcmp(data.ptr(indecies[i]), v) == 0)
			return i;
	}
	auto result = indecies.count;
	indecies.add(data.count);
	auto n = zlen(v);
	data.reserve(data.count + n + 1);
	memcpy(data.ptr(data.count), v, n + 1);
	data.count += n + 1;
	return result;
}

template<> void archive::set<strlib>(strlib& e) {
	set(e.indecies);
	set(e.data);
}