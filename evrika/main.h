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
	unsigned				getcount() const;
	arrayseq*				last();
};
struct database : nameable {
	const bsreq*			type;
	unsigned				size;
	arrayseq*				elements;
	constexpr database() : type(0), size(0), elements(0) {}
	~database();
	void*					add();
	void*					get(int index) const;
	unsigned				getcount() const { return elements->getcount(); }
	unsigned				getsize() const { return size; }
	int						indexof(const void* element) const;
	static bool				readfile(const char* file);
};
struct timestamp {
	unsigned				counter;
	datetime				create_date;
	unsigned char			cluster;
	object_s				type;
	short unsigned			user_id;
	void					generate();
};
struct coreobject {
	timestamp				stamp;
	unsigned				flags;
	datetime				change_date;
	userinfo*				change_user;
	coreobject*				parent;
};
struct reference : coreobject, nameable {
};
struct document : coreobject {
	datetime				date;
	unsigned				number;
	const char*				text;
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
extern adat<userinfo, 2048>	user_data;