#include "crt.h"

#pragma once

class reestr {
	enum type_s : unsigned {
		Number, Text, Struct,
	};
	struct element {
		unsigned		parent;
		unsigned		id;
		type_s			type;
		unsigned		value;
	};
	struct stridx : public array {
		constexpr stridx() : array(sizeof(char)) {}
		unsigned		add(const char* id);
		unsigned		find(const void* p, unsigned c);
		const char*		get(unsigned id) const;
	};
	stridx				strings;
	array				elements;
	unsigned			add(unsigned parent, const char* id, type_s type, unsigned data);
	element*			find(unsigned parent, unsigned id) const;
public:
	constexpr reestr() : strings(), elements(sizeof(element)) {}
	unsigned			add(unsigned parent, const char* id) { return add(parent, id, Struct, 0); }
	unsigned			add(unsigned parent, const char* id, const char* value) { return add(parent, id, Text, strings.add(value)); }
	unsigned			add(unsigned parent, const char* id, int value) { return add(parent, id, Number, value); }
	unsigned			next(unsigned parent, unsigned i) const;
	const char*			getid(unsigned i) { return (i == 0xFFFFFFFF) ? "" : strings.get((((element*)elements.begin()) + i)->id); }
	unsigned			getparent(unsigned i) { return (i == 0xFFFFFFFF) ? i : (((element*)elements.begin()) + i)->parent; }
};