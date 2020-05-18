#include "main.h"

tileset* variant::getresource() const {
	if(type!=Tileset)
		return 0;
	return bsdata<tileset>::elements + value;
}

variant::variant(variant_s t, const array& source, const void* v) {
	auto i = source.indexof(v);
	if(i == -1) {
		type = NoVariant;
		value = 0;
	} else {
		type = t;
		value = i;
	}
}