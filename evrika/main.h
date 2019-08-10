#include "xface/datetime.h"
#include "xface/point.h"
#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/valuelist.h"

#pragma once

struct metadata;

struct nameable {
	const char*				id;
	const char*				name;
	const char*				text;
	constexpr nameable() : id(0), name(0), text(0) {}
	constexpr nameable(const char* id, const char* name) : id(id), name(name), text(0) {}
};
struct requisit : nameable {
	const metadata*			type;
	unsigned				offset;
	constexpr requisit() : type(0), offset(0) {}
	constexpr requisit(const char* id, const char* name, metadata* type, unsigned offset) : nameable(id, name), type(type), offset(offset) {}
	constexpr operator bool() const { return id != 0; }
	constexpr int			get(void* object) const { return *((int*)((char*)object + offset)); }
	constexpr unsigned		getsize() const { return 4; }
	constexpr void*			ptr(void* object) const { return (char*)object + offset; }
};
struct datalist {
	void*					elements;
	unsigned				count;
	unsigned				maximum;
	unsigned				size;
	constexpr datalist() : elements(0), count(0), maximum(0), size(0) {}
	constexpr datalist(unsigned size, unsigned maximum = 64) : elements(0), count(0), maximum(maximum), size(size) {}
	constexpr datalist(void* elements, unsigned count, unsigned size) : elements(elements), count(count), maximum(0), size(size) {}
	void*					add();
	char*					begin() const { return (char*)elements; }
	char*					end() const { return (char*)elements + size*count; }
	void*					get(unsigned index) { return (char*)elements + index * size; }
};
struct metadata : nameable, datalist {
	const metadata*			parent;
	datalist				requisits;
	explicit operator bool() const { return elements != 0; }
	constexpr metadata() : requisits(sizeof(requisit)), parent(0) {}
	constexpr metadata(const char* id, const char* name) : nameable(id, name), parent(0) {}
	constexpr metadata(const char* id, const char* name, const metadata* parent) : nameable(id, name), parent(parent), requisits(sizeof(requisit), 16) {}
	~metadata();
	void*					add() { return datalist::add(); }
	void					add(const requisit* type);
	requisit*				add(const char* id, const char* name, const metadata* type);
	requisit*				addnum(const char* id, const char* nam);
	requisit*				addtxt(const char* id, const char* nam);
	void*					find(unsigned offset, const void* object, unsigned object_size) const;
	static metadata*		find(const void* object);
	static void				select(valuelist& result, bool standart_types, bool object_types);
	void*					get(int index) const { return elements ? (char*)elements + index * size : 0; }
	unsigned				getcount() const { return count; }
	unsigned				getsize() const { return size; }
	static void				initialize();
	static bool				readfile(const char* file);
	static bool				writefile(const char* file);
};