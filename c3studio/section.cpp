#include "section.h"

extern "C" void* malloc(unsigned size);
extern "C" void* realloc(void *ptr, unsigned size);
extern "C" void	free(void* pointer);

section::~section() {
	clear();
}

void section::clear() {
	if(data && count_maximum > minimal_count) {
		free(data);
		data = 0;
		count_maximum = 0;
	}
	count = 0;
}

void section::add(unsigned char v) {
	if(count >= count_maximum) {
		if(count_maximum <= minimal_count) {
			data = (unsigned char*)malloc(minimal_count * 2);
			count_maximum = minimal_count;
		} else {
			unsigned n;
			if(count_maximum >= 256 * 256 * 8)
				n = count_maximum + 256 * 256;
			else
				n = count_maximum * 2;
			auto p = (unsigned char*)realloc(data, n);
			if(!p) {
				clear();
				return;
			} else {
				data = p;
				count_maximum = n;
			}
		}
	}
	data[count++] = v;
}