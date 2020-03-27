#include "main.h"

INSTDATA(cultivatedi) = {{1, "India", 1891761, 57, 1753694, 52, 138067, 4, 1395502, 43, 3287263, 2011},
{2, "United States", 1681826, 17, 1652028, 16, 29798, 0, 8151691, 82, 9833517, 2011},
{3, "China", 1238013, 12, 1084461, 11, 153552, 1, 8358947, 87, 9596960, 2011},
{4, "Russia", 1265267, 7, 1248169, 7, 17098, 0, 15832975, 92, 17098242, 2011},
};
INSTMETA(cultivatedi) = {
	BSREQ(rang),
	BSREQ(name),
	BSREQ(cult_land),
	BSREQ(cult_land_percent),
	BSREQ(arable_land),
	BSREQ(arable_land_percent),
{}};
INSTELEM(cultivatedi)