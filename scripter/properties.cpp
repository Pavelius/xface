#include "main.h"

using namespace code;

template<class T> const char* getnm(const void* object, stringbuilder& sb) {
	return ((T*)object)->name;
}

DGINF(metatypei) = {{"������������", DGREQ(id)},
{}};
DGINF(metadata) = {{"������������", DGREQ(id)},
{"���", DGREQ(type)},
{"#chk �����", DGREQ(flags), {getnm<metatypei>}},
{}};