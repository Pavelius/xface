#include "bsreq.h"
#include "datetime.h"
#include "reestr.h"

#pragma once

enum alignment_s : unsigned char {
	LawfulGood, NeutralGood, ChaoticGood,
	LawfulNeutral, TrueNeutral, ChaoticNeutral,
	LawfulEvil, NeutralEvil, ChaoticEvil,
};
enum gender_s : unsigned char {
	NoGender, Male, Female,
};
struct genderi {
	const char*		id;
	const char*		name;
};
struct alignmenti {
	const char*		id;
	const char*		name;
};
struct cultivatedi {
	char			rang;
	const char*		name;
	unsigned		cult_land;
	unsigned		cult_land_percent;
	unsigned		arable_land;
	unsigned		arable_land_percent;
	unsigned		permanent_crops;
	unsigned		permanent_crops_percent;
	unsigned		other_land;
	unsigned		other_land_percent;
	unsigned		total;
	unsigned		year;
};
struct rowelement {
	const char*		name;
	gender_s		gender;
	alignment_s		alignment;
	char			image;
	char			age;
	unsigned		flags;
	datetime		date;
};
struct element {
	const char*		surname;
	const char*		name[2];
	int				mark;
	char			radio;
	char			age;
	rowelement*		refery[16];
	gender_s		gender;
	alignment_s		alignment;
};