#include "collection.h"

#pragma once

#define	BSREQ(c, f, t) {#f, (unsigned)&((c*)0)->f,\
bsreq::isize<decltype(c::f)>::value,\
sizeof(c::f),\
bsreq::icount<decltype(c::f)>::value,\
t,\
bsreq::iref<decltype(c::f)>::value,\
bsreq::isubtype<decltype(c::f)>::value}

// Metadata field descriptor
struct bsreq {
	enum subtype_s : unsigned char { Scalar, Enum, ADat, ARef, ARem, CFlags };
	// Get count of reference
	template<class T> struct iref : static_int<0> {};
	template<class T> struct iref<T*> : static_int<1 + iref<T>::value> {};
	template<class T, int N> struct iref<T[N]> : static_int<iref<T>::value> {};
	template<class T> struct iref<T[]> : static_int<iref<T>::value> {};
	// Get type size
	template<class T> struct isize : static_int<sizeof(T)> {};
	template<class T, unsigned N> struct isize<T[N]> : static_int<sizeof(T)> {};
	template<class T> struct isize<T[]> : static_int<sizeof(T)> {};
	// Get type count
	template<class T> struct icount : static_int<1> {};
	template<class T, unsigned N> struct icount<T[N]> : static_int<N> {};
	template<class T> struct icount<T[]> : static_int<0> {};
	template<class T, unsigned N> struct icount<adat<T, N>> : static_int<N> {};
	// Get subtype
	template<class T> struct isubtype : static_value<subtype_s, __is_enum(T) ? Enum : Scalar> {};
	template<class T> struct isubtype<T*> : static_value<subtype_s, isubtype<T>::value> {};
	template<class T, unsigned N> struct isubtype<T[N]> : static_value<subtype_s, isubtype<T>::value> {};
	template<class T, unsigned N> struct isubtype<adat<T, N>> : static_value<subtype_s, ADat> {};
	template<class T> struct isubtype<aref<T>> : static_value<subtype_s, ARef> {};
	template<class T, class DT> struct isubtype<cflags<T, DT>> : static_value<subtype_s, CFlags> {};
	const char*			id; // field identifier
	unsigned			offset; // offset from begin of class or object
	unsigned			size; // size of single element
	unsigned			lenght; // total size in bytes of all field (array has size*count)
	unsigned			count; // count of elements
	const bsreq*		type; // metadata of element
	unsigned char		reference; // 1+ if reference
	subtype_s			subtype; // name of subtype (like 'enum') or emphty string for scalar
	//
	operator bool() const { return id != 0; }
	//
	const bsreq*		find(const char* name) const;
	const bsreq*		find(const char* name, unsigned count) const;
	const bsreq*		find(const char* name, const bsreq* type) const;
	int					get(const void* p) const;
	const bsreq*		getkey() const;
	bool				issimple() const { return type == 0; }
	bool				match(const void* p, const char* name) const;
	inline const char*	ptr(const void* data) const { return (const char*)data + offset; }
	inline const char*	ptr(const void* data, int index) const { return (const char*)data + offset + index * size; }
	void				set(const void* p, int value) const;
};
extern bsreq			number_type[]; // standart integer value
extern bsreq			text_type[]; // stantart zero ending string
struct bsval {
	const bsreq*		type;
	void*				data;
	constexpr bsval() : type(0), data(0) {}
	constexpr bsval(void* data, const bsreq* type) : type(type), data(data) {}
	explicit operator bool() const { return data != 0; }
	bsval&				dereference();
	int					get() const { return type->get(type->ptr(data)); }
	bsval&				get(const char* id);
	const char*			getname() const;
	void				set(int value) const { type->set(type->ptr(data), value); }
private:
	const char*			findpart(const char* id);
};