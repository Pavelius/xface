#pragma once

class anyval {
	void*				data;
	unsigned			size;
	unsigned char		bit_offset;
	unsigned char		bit_size;
public:
	constexpr anyval() : data(0), size(0), bit_size(0), bit_offset(0) {}
	constexpr anyval(const anyval& v) : data(v.data), size(v.size), bit_size(0), bit_offset(0) {}
	constexpr anyval(void* data, unsigned size) : data(data), size(size), bit_size(0), bit_offset(0) {}
	constexpr anyval(void* data, unsigned size, unsigned bit_offset, unsigned bit_size) : data(data), size(size), bit_size(bit_size), bit_offset(bit_offset) {}
	template<class T> constexpr anyval(T& v) : data(&v), size(sizeof(T)), bit_size(0), bit_offset(0) {}
	constexpr explicit operator bool() const { return data != 0; }
	operator int() const;
	constexpr bool operator==(const anyval& v) const { return data==v.data && size==v.size
		&& bit_offset==v.bit_offset && bit_size == v.bit_size; }
	void operator=(const int v) const;
	void				clear() { data = 0; size = 0; }
	template<class T> constexpr bool is(const T& v) const { return data==&v; }
	constexpr const void* ptr() const { return data; }
};
