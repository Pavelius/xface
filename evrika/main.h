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
	NoType,
	Enumerator, Reference, Document, Header, User,
};
enum user_s : unsigned char {
	Administrator,
};
struct database {
	constexpr database() : data(0), size(0), count_maximum(0), count(0), next(0) {}
	~database();
	void*					add();
	void*					add(const void* element);
	void					clear();
	void*					get(int index) const;
	unsigned				getmaxcount() const;
	unsigned				getcount() const;
	unsigned				getsize() const { return size; }
	int						indexof(const void* element) const;
	void*					insert(int index, const void* element);
	void					remove(int index, int elements_count);
	void					setup(unsigned size);
private:
	database*				next;
	void*					data;
	unsigned				size;
	unsigned				count;
	unsigned				count_maximum;
	database*				last();
};
struct timestamp {
	datetime				create_date;
	void					generate();
	constexpr bool operator==(const timestamp& e) const {
		return counter == e.counter
			&& create_date == e.create_date
			&& user_id == e.user_id
			&& cluster == e.cluster
			&& type == e.type;
	}
private:
	object_s			type;
	unsigned char		cluster;
	short unsigned		user_id;
	unsigned			counter;
};
struct coreobject {
	unsigned			flags;
	coreobject*			parent;
	datetime			change_date;
	userinfo*			change_user;
private:
	timestamp			stamp;
};
struct reference : coreobject {
	const char*			name;
	const char*			text;
};
struct document : coreobject {
	datetime			date;
	unsigned			number;
	const char*			text;
};
struct enumerator : reference {
	const char*			id;
};
struct headerinfo : reference {
};
struct userinfo : reference {
	const char*			first_name;
	const char*			last_name;
	const char*			third_name;
	const char*			password;
};
extern userinfo*		current_user;
extern adat<userinfo, 2048>	user_data;