#include "crt.h"

#pragma once

class strlib {
	vector<unsigned> indecies;
	vector<char>	data;
	friend struct archive;
public:
	unsigned		add(const char* value);
	const char*		get(unsigned value) const { return (value < indecies.count) ? data.begin() + indecies[value] : ""; }
	unsigned		getcount() const { return indecies.getcount(); }
	unsigned		getoffset(unsigned value) const { return indecies[value]; }
	unsigned		getsize() const { return sizeof(unsigned)*indecies.getcount() + data.getcount(); }
};