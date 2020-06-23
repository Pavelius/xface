#include "datetime.h"
#include "evrika.h"
#include "point.h"
#include "crt.h"
#include "valuelist.h"

#pragma once

enum base_s : unsigned char {
	NumberType, DateType, DateTimeType, TextType,
	ReferenceType, DocumentType, UserType,
	HeaderType,
};
class datastore {
	struct element {
		void*				data;
		unsigned			count;
		unsigned			count_maximum;
		element*			next;
		constexpr void*		ptr(int index, unsigned size) const { return (char*)data + index*size; }
	};
	element*				first;
	unsigned				size;
public:
	constexpr datastore() : first(0), size(0) {}
	void*					add();
	void*					ptr(unsigned index) const;
};
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
	void*					find(const void* object, unsigned size) const;
	void*					get(unsigned index) { return (char*)elements + index * size; }
};
template<class T>
struct datalistc : datalist {
	constexpr datalistc() : datalist(sizeof(T)) {}
	T*						add() { return static_cast<T*>(datalist::add()); }
	T*						begin() const { return (T*)elements; }
	T*						end() const { return (T*)elements + count; }
	T*						ptr(unsigned index) { return (T*)elements + index; }
};
struct requisit {
	const char*				id;
	const char*				name;
	unsigned				offset;
	base_s					type;
	constexpr requisit() : id(0), name(0), type(NumberType), offset(0) {}
	constexpr requisit(const char* id, const char* name, base_s type, unsigned offset) : id(id), name(name), type(type), offset(offset) {}
	constexpr operator bool() const { return id != 0; }
	const requisit*			find(const char* id, unsigned size) const;
	constexpr int			get(void* object) const { return *((int*)((char*)object + offset)); }
	constexpr unsigned		getsize() const { return 4; }
	bool					isreference() const { return type >= ReferenceType; }
	constexpr void*			ptr(void* object) const { return (char*)object + offset; }
	void					set(void* object, int value) const { *((int*)object) = value; }
};
struct requisita : datalistc<requisit> {
	const requisit*			find(const char* id, unsigned size) const;
	const char*				gets(const void* p, const char* url) const;
	void*					ptr(void* object, const char* url, const requisit** result = 0) const;
	void					set(void* object, const char* id, int v) const;
	void					set(void* object, const char* id, const char* v) const;
};
struct metadata : datalist {
	const char*				id;
	const char*				name;
	requisita				requisits;
	explicit operator bool() const { return elements != 0; }
	constexpr metadata() : id(0), name(0), requisits() {}
	constexpr metadata(const char* id, const char* name) : id(id), name(name), requisits() {}
	~metadata();
	void*					add() { return datalist::add(); }
	void					add(const requisit* type);
	requisit*				add(const char* id, const char* name, base_s type);
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
extern metadata				databases[HeaderType + 1];
struct stamp {
	datetime				create_date;
	base_s					type;
	unsigned char			session;
	short unsigned			counter;
	constexpr stamp(base_s type = NumberType) : type(type), create_date(0), session(0), counter(0) {}
	metadata&				getmeta() const { return databases[type]; }
	constexpr bool			isnew() const { return create_date != 0; }
};
struct object : stamp {
	void*					pointer;
	unsigned char			reserved[128 * 4];
	void					read();
	void					write();
};