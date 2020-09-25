#include "main.h"

using namespace code;

template<class T> const char* getnm(const void* object, stringbuilder& sb) {
	return ((T*)object)->name;
}

DGINF(metatypei) = {{"Наименование", DGREQ(id)},
{}};
DGINF(metadata) = {{"Наименование", DGREQ(id)},
{"Тип", DGREQ(type)},
{"#chk Флаги", DGREQ(flags), {getnm<metatypei>}},
{}};