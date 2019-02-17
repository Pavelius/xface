#include "xface/collection.h"

#pragma once

struct metadata;
struct requisit;

extern metadata void_meta[];
extern metadata int_meta[];
extern metadata sint_meta[];
extern metadata usint_meta[];
extern metadata char_meta[];
extern metadata text_meta[];
extern metadata proc_meta[];
extern metadata section_meta[];

metadata*				addtype(const char* id);
metadata*				findtype(const char* id);

struct metadata {
	const char*			id;
	metadata*			type;
	unsigned			size;
	constexpr operator bool() const { return id != 0; }
	static metadata*	add(const char* id);
	requisit*			add(const char* id, metadata* type);
	metadata*			dereference();
	bool				ispointer() const { return id[0] == '*' && id[1] == 0; }
	requisit*			find(const char* id) const;
	metadata*			reference();
};
struct requisit {
	const char*			id;
	metadata*			parent;
	metadata*			type;
	unsigned			offset;
	unsigned			count;
	constexpr operator bool() const { return id != 0; }
	constexpr unsigned getsize() const { return type ? type->size : 0; }
	constexpr unsigned getsizeof() const { return getsize() * count; }
};
struct method {
	const char*			id;
	metadata*			parent;
	metadata*			type;
	unsigned			offset;
	constexpr operator bool() const { return id != 0; }
};
void					choose_metadata(metadata* v);
void					logmsg(const char* format, ...);
void					run_main();
extern adat<requisit, 256 * 16>	requisits;
extern adat<requisit, 256 * 16>	methods;
extern adat<metadata, 256 * 4>	types;