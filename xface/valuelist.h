#include "collection.h"

#pragma once

struct sprite;
struct listelement {
	unsigned char		type;
	unsigned char		image;
	const char*			text;
	int					value;
};
struct valuelist : arem<listelement> {
	listelement&		add(const char* text, int value = 0, unsigned char type = 0, unsigned char image = 0) {
		auto p = arem<listelement>::add();
		p->text = text;
		p->value = value;
		p->image = image;
		p->type = type;
		return *p;
	}
	int					findvalue(int value) const;
};
listelement*			choose(int x, int y, int width, valuelist& vs, const char* start_filter = 0, const sprite* images = 0);