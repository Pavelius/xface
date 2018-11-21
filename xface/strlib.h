#include "collection.h"

#pragma once

struct strlib {
	arem<unsigned>	indecies;
	arem<char>		data;
	unsigned		add(const char* value);
	const char*		get(unsigned value) const { if(value < indecies.count) return data.data + indecies[value]; return ""; }
};