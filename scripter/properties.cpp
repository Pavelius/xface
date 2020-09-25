#include "main.h"

using namespace code;

template<class T> const char* getnm(const void* object, stringbuilder& sb) {
	return ((T*)object)->name;
}
template<> const char* getnm<metadata>(const void* object, stringbuilder& sb) {
	auto p = (metadata*)object;
	p->getname(sb);
	return sb;
}
static bool allow_base_type(const void* object, int index) {
	auto& e = bsdata<metadata>::elements[index];
	if(e.isreference() || e.isarray() || e.ispredefined())
		return false;
	return true;
}
static bool allow_type(const void* object, int index) {
	auto& e = bsdata<metadata>::elements[index];
	if(e.isreference() || e.isarray())
		return false;
	return true;
}

DGINF(metatypei) = {{"������������", DGREQ(id)},
{}};
DGINF(metadata) = {{"������������", DGREQ(id)},
{"���", DGREQ(type), {getnm<metadata>, allow_base_type}},
{"#chk �����", DGREQ(flags), {getnm<metatypei>}},
{}};
DGINF(requisit) = {{"������������", DGREQ(id)},
{"���", DGREQ(type), {getnm<metadata>, allow_type}},
{"����������", DGREQ(count)},
{"#chk �����", DGREQ(flags), {getnm<metatypei>}},
{}};