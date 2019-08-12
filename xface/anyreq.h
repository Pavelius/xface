#pragma once

struct anyreq {
	unsigned				offset;
	unsigned				size;
	constexpr anyreq() : offset(0), size(0) {}
	constexpr anyreq(unsigned offset) : offset(offset), size(4) {}
	constexpr anyreq(unsigned offset, unsigned size) : offset(offset), size(size) {}
	int						get(void* object) const;
	const char*				gets(void* object) const;
	constexpr char*			ptr(void* object) const { return (char*)object + offset; }
	void					set(void* object, int value) const;
};
