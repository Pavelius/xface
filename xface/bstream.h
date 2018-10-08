#pragma once

struct bstream {
	unsigned char*		data;
	unsigned			pos;
	unsigned			maximum;
	//
	bstream(unsigned char* data, unsigned maximum_bytes);
	//
	operator bool() const { return pos < maximum; }
	unsigned			bit(unsigned pos);
	unsigned			get();
	unsigned			get(int size);
	void				put(int value, int size);
};