#pragma once

class anyval {
	void*				data;
	unsigned			size;
	int					value;
public:
	constexpr anyval() : data(0), size(0), value(0) {}
	constexpr anyval(void* data, unsigned size, int value) : data(data), size(size), value(value) {}
	constexpr anyval(const anyval& v) : data(v.data), size(v.size), value(v.value) {}
	template<class T> constexpr anyval(T& v) : data(&v), size(sizeof(T)), value(0) {}
	template<class T> constexpr anyval(T& v, int value) : data(&v), size(sizeof(T)), value(value) {}
	constexpr explicit operator bool() const { return data != 0; }
	constexpr bool operator==(const anyval& v) const { return data==v.data && value==v.value; }
	void				clear() { data = 0; size = 0; value = 0; }
	int					get() const;
	constexpr void*		getptr() const { return data; }
	constexpr unsigned	getsize() const { return size; }
	constexpr int		getvalue() const { return value; }
	void				set(const int v) const;
};