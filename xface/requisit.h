#include "strlib.h"

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
	constexpr unsigned getlenght() const { return size * count; }
};
class manager {
	strlib			strings;
	vector<classtype> classes;
	vector<requisit> requisits;
public:
	unsigned		add(unsigned parent, const char* name, unsigned type, unsigned count = 1, unsigned size = 0);
	unsigned		create(const char* id);
	unsigned		dereference(unsigned v) const;
	unsigned		get(const char* v) { return strings.add(v); }
	unsigned		getsize(unsigned v) const;
	bool			ispredefined(unsigned v) const { return v >= Number; }
	bool			isreference(unsigned v) const;
	unsigned		reference(unsigned v);
};
}