#pragma once

struct anyval {
	void*				data;
	unsigned			size;
	int					value;
	constexpr anyval() : data(0), size(0), value(0) {}
	constexpr anyval(void* data, unsigned size, int value) : data(data), size(size), value(value) {}
	template<class T> constexpr anyval(T& v) : data(&v), size(sizeof(T)), value(0) {}
	template<class T> constexpr anyval(T& v, int value) : data(&v), size(sizeof(T)), value(value) {}
	template<> anyval(const anyval& v) : data(v.data), size(v.size), value(v.value) {}
	template<> anyval(anyval& v) : data(v.data), size(v.size), value(v.value) {}
	constexpr explicit operator bool() const { return data != 0; }
	constexpr bool operator==(const anyval& v) const { return data==v.data && size==v.size && value==v.value; }
	void				clear() { data = 0; size = 0; value = 0; }
	int					get() const;
	template<class T> constexpr bool is(const T& v) const { return data==&v; }
	void				set(const int v) const;
};