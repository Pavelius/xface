#include "crt.h"

#pragma once

struct stridx : public array {
	constexpr stridx() : array(sizeof(char)) {}
	unsigned			add(const char* id);
	unsigned			find(const void* p, unsigned c);
};

struct reestr {
	enum type_s : unsigned {
		Number, Text, Struct,
	};
	struct element {
		unsigned		id;
		type_s			type;
		unsigned		data;
	};
	stridx				strings;
	array				elements;
};