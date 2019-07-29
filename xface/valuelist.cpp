#include "valuelist.h"

int valuelist::findvalue(int value) const {
	for(auto& e : *this) {
		if(e.value == value)
			return &e - data;
	}
	return -1;
}

listelement& valuelist::add(const char* text, int value, unsigned char type, unsigned char image) {
	auto p = adat::add();
	p->text = text;
	p->value = value;
	p->image = image;
	p->type = type;
	return *p;
}