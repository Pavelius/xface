#include "collection.h"

#pragma once

namespace compiler {
struct string : arem<char> {
	arem<unsigned>	index;
	constexpr string() : arem(), index() {}
	unsigned		add(const char* value);
	const char*		get(unsigned value) const;
};
struct requisit {
	unsigned		id;
	requisit*		parent;
	requisit*		type;
	unsigned		count;
	unsigned		size;
	unsigned		offset;
	void			clear();
	requisit*		dereference() const;
	bool			isobject() const;
};
extern requisit		number[];
extern requisit		object[];
extern requisit		pointer[];
extern requisit		text[];
struct manager {
	string			strings;
	arem<requisit>	requisits;
	requisit*		add(requisit* parent, const char* name, requisit* type);
	requisit*		create(const char* id);
	requisit*		reference(const requisit* req);
};
}