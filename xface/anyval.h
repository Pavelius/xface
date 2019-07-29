#pragma once

struct anyval {
	void*				data;
	unsigned			size;
	constexpr anyval() : data(0), size(0) {}
	constexpr anyval(const anyval& v) : data(v.data), size(v.size) {}
	constexpr anyval(void* data, unsigned size) : data(data), size(size) {}
	template<class T> constexpr anyval(T& v) : data(&v), size(sizeof(T)) {}
	constexpr explicit operator bool() const { return data != 0; }
	constexpr bool operator==(const anyval& v) const { return data==v.data && size==v.size; }
	void				clear() { data = 0; size = 0; }
	int					get() const;
	template<class T> constexpr bool is(const T& v) const { return data==&v; }
	constexpr const void* ptr() const { return data; }
	void				set(const int v) const;
};
