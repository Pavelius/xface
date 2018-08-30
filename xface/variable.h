#pragma once

struct variable {
	void*		data;
	unsigned	size;
	constexpr variable() : data(0), size(0) {}
	constexpr variable(void* data, unsigned size) : data(data), size(size) {}
	template<typename T> constexpr variable(T& value) : data(&value), size(sizeof(value)) {}
	int			get() const;
	void		set(int value) const;
};