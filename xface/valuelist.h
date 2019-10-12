#include "crt.h"

#pragma once

struct sprite;

struct listelement {
	const char*			text;
	int					value;
	unsigned char		type;
	unsigned char		image;
};
struct valuelist : adat<listelement, 64> {
	listelement&		add(const char* text, int value = 0, unsigned char type = 0, unsigned char image = 0);
	listelement*		choose(int x, int y, int width, const char* start_filter = 0, const sprite* images = 0);
	int					findvalue(int value) const;
};