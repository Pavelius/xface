#include "xface/datetime.h"
#include "xface/point.h"
#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/valuelist.h"

#pragma once

struct metadata;

struct datalist {
	void*					elements;
	unsigned				count;
	unsigned				maximum;
	unsigned				size;
	constexpr datalist() : elements(0), count(0), maximum(0), size(0) {}
	constexpr datalist(unsigned size) : elements(0), count(0), maximum(maximum), size(size) {}
	void*					add();
	char*					begin() const { return (char*)elements; }
	char*					end() const { return (char*)elements + size * count; }
	void*					get(unsigned index) { return (char*)elements + index * size; }
};
struct requisit {
	const char*				id;
	const char*				name;
	const metadata*			type;
	unsigned				offset;
	constexpr requisit() : id(0), name(0), type(0), offset(0) {}
	constexpr requisit(const char* id, const char* name, metadata* type, unsigned offset) : id(id), name(name), type(type), offset(offset) {}
	constexpr operator bool() const { return id != 0; }
	const requisit*			find(const char* id, unsigned size) const;
	constexpr int			get(void* object) const { return *((int*)((char*)object + offset)); }
	constexpr unsigned		getsize() const { return 4; }
	bool					isreference() const;
	constexpr void*			ptr(void* object) const { return (char*)object + offset; }
	void*					ptr(void* object, const char* url, const requisit** result = 0) const;
};
struct metadata : datalist {
	const char*				id;
	const char*				name;
	datalist				requisits;
	explicit operator bool() const { return elements != 0; }
	constexpr metadata() : id(0), name(0), requisits(sizeof(requisit)) {}
	constexpr metadata(const char* id, const char* name) : id(id), name(name), requisits(sizeof(requisit)) {}
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