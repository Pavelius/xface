#include "xface/bsreq.h"
#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/datetime.h"

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
template<> struct bsmeta<datetime> : bsmeta<int> {};
DECLENUM(alignment);
DECLENUM(gender);