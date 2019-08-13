#pragma once

#define ANOFS(c, f) ((unsigned)&((c*)0)->f)
#define ANREQ(c, f) anyreq(ANOFS(c,f), sizeof(c::f))
#define ANBIT(c, f, b) anyreq(ANOFS(c,f), sizeof(c::f), b)

struct anyreq {
	unsigned short			offset;
	unsigned char			size;
	unsigned char			bit;
	constexpr anyreq() : offset(0), size(0), bit(0) {}
	constexpr anyreq(unsigned short offset, unsigned char size) : offset(offset), size(size), bit(0) {}
	constexpr anyreq(unsigned short offset, unsigned char size, unsigned char bit) : offset(offset), size(size), bit(bit) {}
	constexpr bool operator== (const anyreq& e) { return e.offset == offset; }
	int						get(void* object) const;
	const char*				gets(void* object) const;
	constexpr char*			ptr(void* object) const { return (char*)object + offset; }
	void					set(void* object, int value) const;
};
