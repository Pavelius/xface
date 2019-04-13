#include "xface/datetime.h"
#include "xface/point.h"
#include "xface/bsdata.h"
#include "xface/bsreq.h"
#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/stringcreator.h"

#pragma once

struct reference;
struct document;
struct userinfo;
struct headerinfo;

enum object_s : unsigned char {
	Reference, Document, Header, User,
};
struct nameable {
	const char*				id;
	const char*				name;
	const char*				text;
	constexpr nameable() : id(0), name(0), text(0) {}
};
struct arrayseq {
	unsigned				count;
	unsigned				count_maximum;
	arrayseq*				next;
	char*					begin() const { return (char*)this + sizeof(arrayseq); }
	unsigned				getcount() const;
	arrayseq*				last();
};
struct database : nameable {
	unsigned				size;
	arrayseq*				elements;
	constexpr database() : size(0), elements(0) {}
	explicit operator bool() const { return elements != 0; }
	~database();
	void*					add();
	void*					find(unsigned offset, const void* object, unsigned size) const;
	void*					get(int index) const;
	unsigned				getcount() const { return elements->getcount(); }
	unsigned				getsize() const { return size; }
	int						indexof(const void* element) const;
	static bool				readfile(const char* file);
	static bool				writefile(const char* file);
};
extern database				databases[256];
struct timestamp {
	object_s				type;
	unsigned char			session;
	short unsigned			counter;
	datetime				create_date;
	constexpr timestamp() : type(Reference), session(0), counter(0), create_date(0) {}
	constexpr timestamp(object_s type) : type(type), session(0), counter(0), create_date(0) {}
	timestamp*				getreference() const { return (timestamp*)databases[type].find(0, this, sizeof(*this)); }
	void					write();
};
struct coreobject : timestamp {
	unsigned				flags;
	datetime				change_date;
	userinfo*				change_user;
	coreobject*				parent;
	constexpr coreobject(object_s type) : timestamp(type), flags(0), change_date(0), change_user(0), parent(0) {}
	coreobject*				write(); // Store element to database and get valid reference
};
struct reference : coreobject, nameable {
};
struct document : coreobject {
	datetime				date;
	unsigned				number;
	const char*				text;
	constexpr document() : coreobject(Document), date(0), number(0), text(0) {}
};
struct headerinfo : reference {
};
struct userinfo : reference {
	const char*				first_name;
	const char*				last_name;
	const char*				third_name;
	const char*				password;
};
extern userinfo*			current_user;