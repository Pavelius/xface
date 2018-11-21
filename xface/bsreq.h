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
	template<int V> struct ival { static constexpr int value = V; };
	// Get count of reference
	template<class T> struct iref : ival<0> {};
	template<class T> struct iref<T*> : ival<1 + iref<T>::value> {};
	template<class T, int N> struct iref<T[N]> : ival<iref<T>::value> {};
	template<class T> struct iref<T[]> : ival<iref<T>::value> {};
	// Get type size
	template<class T> struct isize : ival<sizeof(T)> {};
	template<class T, unsigned N> struct isize<T[N]> : ival<sizeof(T)> {};
	template<class T> struct isize<T[]> : ival<sizeof(T)> {};
	// Get type count
	template<class T> struct icount : ival<1> {};
	template<class T, unsigned N> struct icount<T[N]> : ival<N> {};
	template<class T> struct icount<T[]> : ival<0> {};
	template<class T, unsigned N> struct icount<adat<T, N>> : ival<N> {};
	// Get subtype
	template<class T> struct isubtype { static constexpr subtype_s value = __is_enum(T) ? Enum : Scalar; };
	template<class T> struct isubtype<T*> { static constexpr subtype_s value = isubtype<T>::value; };
	template<class T, unsigned N> struct isubtype<T[N]> { static constexpr subtype_s value = isubtype<T>::value; };
	template<class T, unsigned N> struct isubtype<adat<T, N>> { static constexpr subtype_s value = ADat; };
	template<class T> struct isubtype<aref<T>> { static constexpr subtype_s value = ARef; };
	template<class T, class DT> struct isubtype<cflags<T, DT>> { static constexpr subtype_s value = CFlags; };
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
	inline const char*	ptr(const void* data, int index) const { return (const char*)data + offset + index*size; }
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