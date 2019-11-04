#include "crt.h"

#pragma once

// Basic autogrow array
struct arraydata {
	struct iterator {
		const arraydata*	source;
		char*				pb;
		char*				pe;
		constexpr bool operator!=(const iterator& e) const { return pb != e.pb; }
		void				initialize(const arraydata* source, unsigned size);
		void				increment(unsigned size);
	};
	unsigned				count;
	unsigned				maximum;
	arraydata*				next;
	constexpr arraydata(unsigned N) : count(0), maximum(N), next(0) {}
	explicit constexpr operator bool() const { return count != 0; }
	void*					add(unsigned size);
	void*					begin() const { return (char*)this + sizeof(*this); }
	void					clear();
	void*					end(unsigned size) { return (char*)this + sizeof(*this) + count * size; }
	unsigned				getcount() const;
	void*					get(int index, unsigned size) const;
	int						indexof(const void* e, unsigned size) const;
	void					release();
};
template<class T, unsigned N = 64>
class agrw : public arraydata {
	T						data[N]; // Размер data[] увеличивается динамически
public:
	typedef T				element;
	constexpr agrw() : arraydata(N), data() {}
	~agrw() { release(); }
	T& operator[](int index) { return *((T*)arraydata::get(index, sizeof(T))); }
	const T& operator[](int index) const { return *((const T*)arraydata::get(index, sizeof(T))); }
	T*						add() { auto p = (T*)arraydata::add(sizeof(T)); *p = T(); return p; }
	T*						get(int index) const { return (T*)arraydata::get(index, sizeof(T)); }
	int						indexof(const T* e) const { return arraydata::indexof(e, sizeof(T)); }
};