#include "color.h"

#pragma once

namespace setting {
enum type_s : unsigned char {
	Number, Text, Bool,
	Radio, Color, Url,
};
typedef bool(*ptest)();
typedef void(*pcall)();
struct reference {
	type_s			type;
	void*			data;
	unsigned		size;
	int				value;
	reference() = default;
	constexpr reference(const reference& v) = default;
	constexpr reference(type_s type, void* data, unsigned size) : type(type), data(data), size(size), value(0) {}
	constexpr reference(type_s type, void* data, unsigned size, int value) : type(type), data(data), size(size), value(value) {}
	template<class T> constexpr reference(T& v) : reference(Number, &v, sizeof(v)) {}
	constexpr reference(const char*& v) : reference(Text, &v, sizeof(v)) {}
	constexpr reference(bool& v) : reference(Bool, &v, sizeof(v)) {}
	constexpr reference(color& v) : reference(Color, &v, sizeof(v)) {}
	int				get() const;
	void*			getptr() const { return data; }
	void			set(int v) const;
};
struct element {
	const char*		name;
	reference		var;
	bool(*ptest)(); // Test visibility
	void(*pcall)();
};
template<class T>
class list {
	const T*		data;
	unsigned		count;
public:
	const T*		begin() { return data; }
	const T*		end() { return data + count; }
	list() = default;
	template<class T, unsigned N> constexpr list(T(&data)[N]) : data(data), count(N) {};
};
struct header {
	const char*		division;
	const char*		page;
	const char*		group;
	list<element>	elements;
	ptest			visible;
	header*			next;
	header(const char* division, const char* page, const char* group, const list<element>& elements, ptest visible = 0);
	static header*	first;
};
}