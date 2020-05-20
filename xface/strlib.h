#include "crt.h"

#pragma once

class strlib {
	arem<unsigned>	indecies;
	arem<char>		data;
	friend struct archive;
public:
	unsigned		add(const char* value);
	const char*		get(unsigned value) const { return (value < indecies.count) ? data.data + indecies[value] : ""; }
	unsigned		getcount() const { return indecies.getcount(); }
	unsigned		getoffset(unsigned value) const { return indecies.data[value]; }
	unsigned		getsize() const { return sizeof(unsigned)*indecies.getcount() + data.getcount(); }
};