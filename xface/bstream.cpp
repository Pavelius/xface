#include "bstream.h"

bstream::bstream(unsigned char* data, unsigned maximum_bytes) : data(data), pos(0), maximum(maximum_bytes*8)
{
}

void bstream::put(int value, int size)
{
	for(int j = size-1; j>=0; j--, pos++)
	{
		if(value&(1<<j))
			data[pos>>3] |= 1<<(7-(pos&(8-1)));
		else
			data[pos>>3] &= ~(1<<(7-(pos&(8-1))));
	}
}

unsigned bstream::get(int size)
{
	int r = 0;
	for(int j = size-1; j>=0; j--, pos++)
	{
		r <<= 1;
		if(data[pos/8] & (1<<(7-(pos&(8-1)))))
			r |= 1;
	}
	return r;
}

unsigned bstream::get()
{
	unsigned r = ((data[pos >> 3] >> (pos & 0x7)) & (unsigned char)1);
	pos++;
	return r;
}

unsigned bstream::bit(unsigned pos)
{
	return ((data[pos >> 3] >> (pos & 0x7)) & (unsigned char)1);
}