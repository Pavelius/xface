#include "xface/datetime.h"
#include "xface/point.h"
#include "xface/bsreq.h"
#include "xface/collection.h"
#include "xface/crt.h"

#pragma once

struct reference;
struct documenti;
struct useri;
struct headeri;

enum type_s : unsigned char {
	Number, Text, Date, DateTime,
	RefObject, Table,
};
enum object_s : unsigned char {
	Reference, Document, Header, Requisit,
	User,
};
struct nameable {
	const char*				id;
	const char*				name;
	const char*				text;
	constexpr nameable() : id(0), name(0), text(0) {}
};
class database : public nameable {
	unsigned				size;
	unsigned				count;
	unsigned				maximum;
	void*					elements;
	struct iterator {
		char*				p;
		unsigned			size;
		constexpr iterator(void* p, unsigned size) : p((char*)p), size(size) {}
		constexpr char& operator*() const { return *p; }
		constexpr bool operator!=(const iterator& e) const { return p != e.p; }
		void operator++() { p += size; }
	};
public:
	constexpr database() : size(0), count(0), maximum(0), elements(0) {}
	explicit operator bool() const { return elements != 0; }
	~database();
	void*					add();
	iterator				begin() const { return iterator(elements, size); }
	iterator				begin() { return iterator(elements, size); }
	iterator				end() { return iterator((char*)elements + count*size, size); }
	void*					find(unsigned offset, const void* object, unsigned object_size) const;
	void*					get(int index) const { return elements ? (char*)elements + index * size : 0; }
	unsigned				getcount() const { return count; }
	unsigned				getsize() const { return size; }
	void					initialize(unsigned size, unsigned maximum);
	static bool				readfile(const char* file);
	static bool				writefile(const char* file);
};
extern database				databases[256];
struct datareq {
	reference*				field;
	unsigned				size;
};
struct stampi {
	unsigned				counter;
	constexpr stampi() : counter(0) {}
	constexpr stampi(object_s type) : counter(type<<24|0xFFFFFF) {}
	bool					isnew() const { return (counter & 0xFFFFFF) == 0xFFFFFF; }
	database&				getbase() const { return databases[gettype()]; }
	unsigned				getbaseindex() const { return (counter & 0xFFFFFF); }
	stampi*					getreference() const { return (stampi*)getbase().find(0, this, sizeof(*this)); }
	constexpr type_s		gettype() const { return type_s(counter>>24); }
	void					write();
};
struct actioni {
	datetime				date;
	useri*					user;
	constexpr actioni() : date(), user() {}
};
struct objecti : stampi {
	objecti*				parent;
	unsigned				flags;
	actioni					create, change;
	constexpr objecti(object_s type) : stampi(type), flags(0), parent(0) {}
	objecti*				write(); // Store element to database and get valid reference
};
struct reference : objecti, nameable {
	constexpr reference() : objecti(Reference) {}
	constexpr reference(object_s type) : objecti(type) {}
};
struct documenti : objecti {
	datetime				date;
	const char*				text;
	constexpr documenti() : objecti(Document), date(0), text(0) {}
};
struct headeri : reference {
	constexpr headeri() : reference(Header) {}
};
struct requisit : reference {
	type_s					type;
	headeri*				type_folder;
	constexpr requisit() : reference(Requisit), type(Number), type_folder() {}
};
struct useri : reference {
	const char*				first_name;
	const char*				last_name;
	const char*				third_name;
	const char*				password;
	constexpr useri() : reference(User), first_name(0), last_name(0), third_name(0), password(0) {}
};
extern useri*				current_user;