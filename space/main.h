#include "xface/crt.h"

#pragma once

enum unit_s : unsigned char {
	NoUnit,
	Raiders, Soldiers, Troopers, Terminators,
};
enum vehicle_s : unsigned char {
	NoVehicle,
	APC, Tank, Shuttle, Helicopter,
};
enum wound_s : unsigned char {
	NoWound, LightWound, HeavyWound, FatalWound
};

template<typename T> struct bsmeta {
	static T			elements[];
};
struct combati {
	char				miss;
	char				light;
	char				heavy;
	char				fatal;
	wound_s				shoot(char bonus = 0) const;
	wound_s				suffer(wound_s w) const;
};
struct weaponi {
	const char*			name;
	combati				combat;
};
struct armori {
	const char*			name;
	combati				combat;
};
struct uniti { // One unit have 10 people
	unit_s				type;
	combati				squad;
	void				apply(wound_s w);
	int					getalive() const;
	int					getlight() const { return squad.light; }
	int					getready() const;
	constexpr explicit operator bool() const { return type != NoUnit; }
};
struct vehicle {
	vehicle_s			type;
	char				hits;
	uniti*				crew;
	constexpr explicit operator bool() const { return type != NoVehicle; }
};