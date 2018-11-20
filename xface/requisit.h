#include "collection.h"

#pragma once

namespace compiler {
struct string {
	arem<char>		data;
	arem<unsigned>	index;
	constexpr string() : data(), index() {}
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
	constexpr requisit() : id(0), parent(0), type(0), count(0), size(0), offset(0) {}
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
	constexpr manager() : strings(), requisits() {}
	requisit*		add(requisit* parent, const char* name, requisit* type);
	requisit*		create(const char* id);
	requisit*		reference(const requisit* req);
	void			write(const char* url);
};
}