#include "main.h"

database databases[256];

void database::initialize(unsigned size, unsigned maximum) {
	if(this->size)
		return;
	this->size = size;
	this->maximum = maximum;
	this->elements = new char[maximum*size];
}

void* database::add() {
	if(count < maximum)
		return (char*)elements + (count++)*size;
	return elements;
}

database::~database() {
	if(elements)
		delete elements;
}

void* database::find(unsigned offset, const void* object, unsigned object_size) const {
	auto pe = (char*)elements + offset + size * count;
	for(auto pp = (char*)elements + offset; pp < pe; pp += size) {
		if(memcmp(pp, object, object_size) == 0)
			return pp - offset;
	}
	return 0;
}