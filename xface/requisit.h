#include "collection.h"

#pragma once

namespace compiler {
enum standart_s : unsigned {
	Number = 0xFFFFFF00, Text, Pointer,
	Null = 0xFFFFFFFF,
};
struct classtype {
	unsigned		id; // Can be null
	unsigned		type; // Can be null
	unsigned		size;
};
struct requisit {
	unsigned		id; // Can be null
	unsigned		parent;
	unsigned		type;
	unsigned		count;
	unsigned		size;
	unsigned		offset;
	unsigned getlenght() const {
		return size * count;
	}
};
struct manager {
	unsigned		add(unsigned parent, const char* name, unsigned type, unsigned count = 1, unsigned size = 0);
	unsigned		create(const char* id);
	unsigned		dereference(unsigned v) const;
	unsigned		get(const char* v);
	unsigned		getsize(unsigned v) const;
	bool			ispredefined(unsigned v) const { return v >= Number; }
	bool			isreference(unsigned v) const;
	unsigned		reference(unsigned v);
private:
	arem<unsigned>	strings;
	arem<classtype>	classes;
	arem<requisit>	requisits;
	arem<char>		section_strings;
};
}