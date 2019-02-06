#include "xface/point.h"
#include "xface/bsdata.h"
#include "xface/bsreq.h"
#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/stringcreator.h"

#pragma once

enum map_s : unsigned char {
	Rectangle
};

struct map_info {
	map_s		type;
	point		element;
	point		size;
};