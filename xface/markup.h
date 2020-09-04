#include "crt.h"
#include "stringbuilder.h"

#pragma once

#define DGLNK(R,T) template<> struct dginf<R> : dginf<T> {};
#define DGINF(T) const markup dginf<T>::meta[]
#define DGGEN(R, T1, T2, MS) {dginf<meta_decoy<T1>::value>::meta,\
bsdata<meta_decoy<T2>::value>::source_ptr,\
(unsigned)&((data_type*)0)->R,\
sizeof(data_type::R),\
MS}
#define DGINH(R) {dginf<meta_decoy<R>::value>::meta,\
0,\
(unsigned)static_cast<R*>((data_type*)0),\
sizeof(R),\
0}
#define DGREQ(R) DGGEN(R, decltype(data_type::R), decltype(data_type::R), 0)
#define DGCHK(R, M) DGGEN(R, decltype(data_type::R), decltype(data_type::R), M)

struct markup;

typedef bool(*fnallow)(const void* object, int index); // Is allow some property
typedef void(*fncommand)(void* object); // Object's actions
typedef int(*fndraw)(int x, int y, int width, const void* object); // Custom draw
typedef const char* (*fntext)(const void* object, stringbuilder& sb);
typedef void(*fnsource)(const void* object, array& source);
typedef bool(*fnchoose)(const void* object, array& source, void* pointer);
typedef bool(*fnvisible)(const void* object);

struct fnlist {
	fntext				getname;
	fnallow				allow;
	fnsource			source;
	fnchoose			choose;
	fndraw				preview;
	int					view_width;
};
struct fnelement {
	fnvisible			visible;
	fntext				getheader;
	fncommand			execute;
};
template<class T> struct dginf {
	typedef T			data_type;
	static const markup	meta[];
};
// Data binding element
struct markitem {
	const markup*		type;
	array*				source;
	unsigned			offset;
	unsigned			size;
	unsigned			mask;
	//
	constexpr bool		isnum() const { return type == dginf<int>::meta; }
	constexpr bool		isreference() const { return source != 0; }
	constexpr bool		istext() const { return type == dginf<const char*>::meta; }
	void*				ptr(void* object) const { return (char*)object + offset; }
};
// Standart markup
struct markup {
	constexpr explicit operator bool() const { return title || value.type; }
	const char*			title;
	markitem			value;
	fnlist				list;
	fnelement			proc;
	//
	bool				is(const char* id) const;
	bool				ischeckboxes() const { return is("chk"); }
	bool				isdecortext() const { return value.type == 0; }
	bool				isgroup() const { return value.type != 0 && !list.getname && !value.istext() && !value.isnum(); }
	bool				ispage() const { return title && title[0] == '#'; }
};
DGLNK(char, int)
DGLNK(short, int)
DGLNK(unsigned char, int)
DGLNK(unsigned short, int)
DGLNK(unsigned, int)
DGLNK(bool, int)
