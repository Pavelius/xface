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
	char				miss, light, heavy, fatal;
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
struct uniti : combati { // One unit have 10 people
	unit_s				type;
	constexpr uniti(unit_s v) : type(v), combati{10, 0, 0, 0} {}
	void				apply(wound_s w);
	//void				create(unit_s v);
	constexpr int		getalive() const { return miss - fatal; }
	constexpr int		getlight() const { return light; }
	constexpr int		getready() const { return getalive() - heavy; }
	constexpr explicit operator bool() const { return fatal < miss; }
};
struct vehicle {
	vehicle_s			type;
	char				hits;
	uniti*				crew;
	constexpr explicit operator bool() const { return type != NoVehicle; }
};