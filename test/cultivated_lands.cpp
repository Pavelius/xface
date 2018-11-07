#include "xface/bsdata.h"

struct cultivated_land {
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
cultivated_land cultivated_land_data[] = {{1, "India", 1891761, 57, 1753694, 52, 138067, 4, 1395502, 43, 3287263, 2011},
{2, "United States", 1681826, 17, 1652028, 16, 29798, 0, 8151691, 82, 9833517, 2011},
{3, "China", 1238013, 12, 1084461, 11, 153552, 1, 8358947, 87, 9596960, 2011},
{4, "Russia", 1265267, 7, 1248169, 7, 17098, 0, 15832975, 92, 17098242, 2011},
};
bsreq cultivated_land_type[] = {
	BSREQ(cultivated_land, rang, number_type),
	BSREQ(cultivated_land, name, text_type),
	BSREQ(cultivated_land, cult_land, number_type),
	BSREQ(cultivated_land, cult_land_percent, number_type),
	BSREQ(cultivated_land, arable_land, number_type),
	BSREQ(cultivated_land, arable_land_percent, number_type),
{}};
BSMETA(cultivated_land);