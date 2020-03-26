#include "crt.h"

#pragma once

class strlib {
	arem<unsigned>	indecies;
	arem<char>		data;
	friend struct archive;
public:
	unsigned		add(const char* value);
	const char*		get(unsigned value) const { if(value < indecies.count) return data.data + indecies[value]; return ""; }
};