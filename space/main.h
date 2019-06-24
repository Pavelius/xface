#pragma once

enum unit_s : unsigned char {
	NoUnit,
	Raiders, Soldiers, Troopers, Terminators,
};
enum vehicle_s : unsigned char {
	NoVehicle,
	APC, Tank, Shuttle, Helicopter,
};

template<typename T> struct bsmeta {
	static T			elements[];
};
struct combati {
	char				miss;
	char				light;
	char				heavy;
	char				fatal;
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
	int					getalive() const;
	int					getlight() const { return squad.light; }
	int					getready() const;
};
struct vehicle {
	vehicle_s			type;
	char				hits;
	uniti*				crew;
};