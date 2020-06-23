#include "crt.h"
#include "datetime.h"

#pragma once

enum base_s : unsigned char {
	NumberType, DateType, DateTimeType, TextType,
	ReferenceType, DocumentType, UserType,
	HeaderType,
};
struct requisit;
union rfid {
	unsigned				value;
	unsigned char			byte[4];
	unsigned short			word[2];
};
struct database : public array {
	requisit*				requisits;
};
extern database				databases[256];
struct stamp {
	unsigned char			base; // Maximum 256 bases
	unsigned char			session_id; // Maximum 255 users working at same time and one user (has 0 ID) is System
	short unsigned			counter; // Unique number in session and auto-increment each time when object create
	datetime				create_date; // Date when object is created
	constexpr stamp() : base(0), create_date(0), session_id(0), counter(0) {}
	constexpr bool			isnew() const { return create_date != 0; }
	void					generate();
	void					generate(base_s id);
};
struct reference : stamp {
	rfid					parent;
	const char*				name;
	const char*				comment;
};
struct requisit : reference {
	const char*				id;
	base_s					type;
	constexpr unsigned		getsize() const { return 4; }
	bool					isreference() const { return type >= ReferenceType; }
};