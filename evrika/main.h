#include "crt.h"
#include "datetime.h"

#pragma once

enum base_s : unsigned char {
	NumberType, DateType, DateTimeType, TextType,
	ReferenceType, DocumentType,
	HeaderType, RequisitType, UserType,
};
struct requisit;
typedef unsigned			rfid;
struct database : public array {
	requisit*				requisits;
};
extern database				databases[256];
constexpr base_s			gtb(unsigned v) { return (base_s)(v >> 24); }
constexpr unsigned			gti(unsigned v) { return v & 0xFFFFFF; }
constexpr unsigned			gtr(base_s b, unsigned v) { return (b << 24) | v; }
struct stamp {
	unsigned char			base; // Maximum 256 bases
	unsigned char			session_id; // Maximum 255 users working at same time and one user (has 0 ID) is System
	short unsigned			counter; // Unique number in session and auto-increment each time when object create
	datetime				create_date; // Date when object is created
	constexpr stamp() : base(0), create_date(0), session_id(0), counter(0) {}
	constexpr bool			isnew() const { return create_date != 0; }
	void					generate();
};
struct reference : stamp {
	rfid					parent;
	rfid					name;
	rfid					comment;
};
struct header : reference {
	base_s					base;
	unsigned				read_permission;
	unsigned				write_permission;
};
struct requisit : reference {
};
struct user : reference {
	rfid					firstname;
	rfid					lastname;
	rfid					surname;
	rfid					read_permission;
	rfid					write_permission;
};
struct keyvalue {
	rfid					key; // parent object (only reference type)
	rfid					requisit; // requisit value
	rfid					value; // object value
};