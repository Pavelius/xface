#include "crt.h"

#pragma once

#define	BSREQ(fn) {#fn, (unsigned)&((data_type*)0)->fn,\
sizeof(meta_decoy<decltype(data_type::fn)>::value),\
sizeof(data_type::fn),\
meta_count<decltype(data_type::fn)>::value,\
bsmeta<meta_decoy<decltype(data_type::fn)>::value>::meta,\
meta_kind<decltype(data_type::fn)>::value,\
bsdata<meta_decoy<decltype(data_type::fn)>::value>::source_ptr}
#define BSDATA(c, i) c bsmeta<c>::elements[i];\
bsdatat<c> bsmeta<c>::data(#c, bsmeta<c>::elements, KindScalar);
#define assert_enum(e, last) static_assert(sizeof(bsdata<e##i>::elements) / sizeof(bsdata<e##i>::elements[0]) == last + 1, "Invalid count of " #e " elements");\
template<> const bsreq bsmeta<e##i>::meta[] = {BSREQ(id), BSREQ(name), {}};\
template<> array bsdata<e##i>::source(bsdata<e##i>::elements, sizeof(bsdata<e##i>::elements[0]), sizeof(bsdata<e##i>::elements)/sizeof(bsdata<e##i>::elements[0]));

// Basic metadata types
enum bstype_s : unsigned char {
	KindNoType,
	KindNumber, KindText, KindScalar, KindEnum, KindReference,
	KindADat, KindARef, KindARem, KindCFlags
};
// Get kind
template<class T> struct meta_kind : static_value<bstype_s, __is_enum(T) ? KindEnum : KindScalar> {};
template<> struct meta_kind<const char*> : static_value<bstype_s, KindText> {};
template<> struct meta_kind<char> : static_value<bstype_s, KindNumber> {};
template<> struct meta_kind<short> : static_value<bstype_s, KindNumber> {};
template<> struct meta_kind<int> : static_value<bstype_s, KindNumber> {};
template<> struct meta_kind<unsigned char> : static_value<bstype_s, KindNumber> {};
template<> struct meta_kind<unsigned short> : static_value<bstype_s, KindNumber> {};
template<> struct meta_kind<unsigned int> : static_value<bstype_s, KindNumber> {};
template<> struct meta_kind<bool> : static_value<bstype_s, KindNumber> {};
template<class T> struct meta_kind<T*> : static_value<bstype_s, KindReference> {};
template<class T> struct meta_kind<T**> {}; // Not allowed complex pointer
template<class T> struct meta_kind<const T*> : static_value<bstype_s, KindReference> {};
template<class T> struct meta_kind<const T> : static_value<bstype_s, meta_kind<T>::value> {};
template<class T, unsigned N> struct meta_kind<T[N]> : static_value<bstype_s, meta_kind<T>::value> {};
template<class T, unsigned N> struct meta_kind<adat<T, N>> : static_value<bstype_s, KindADat> {};
template<class T> struct meta_kind<aref<T>> : static_value<bstype_s, KindARef> {};
template<class T, class DT> struct meta_kind<cflags<T, DT>> : static_value<bstype_s, KindCFlags> {};
// Metadata field descriptor
struct bsreq {
	const char*			id; // field identifier
	unsigned			offset; // offset from begin of class or object
	unsigned			size; // size of single element
	unsigned			lenght; // total size in bytes of all field (array has size*count)
	unsigned			count; // count of elements
	const bsreq*		type; // metadata of element
	bstype_s			subtype; // metadata subtype
	array*				source; // data source for enumerators
	//
	constexpr explicit operator bool() const { return id != 0; }
	//
	void*				dereference(const void* data, const bsreq** result) const;
	const bsreq*		find(const char* name) const;
	const bsreq*		find(const char* name, unsigned count) const;
	const bsreq*		find(const char* name, const bsreq* type) const;
	int					get(const void* p) const;
	const char*			get(const void* p, char* result, const char* result_max) const;
	const char*			gets(const void* p) const;
	const bsreq*		getname() const;
	const char*			getmetaname() const; // Extern function
	bool				is(bstype_s v) const { return subtype == v; }
	bool				issimple() const { return is(KindNumber) || is(KindText); }
	bool				match(const void* p, const char* name) const;
	char*				ptr(const void* data) const { return (char*)data + offset; }
	char*				ptr(const void* data, int index) const { return (char*)data + offset + index * size; }
	void*				ptr(const void* data, const char* url, const bsreq** result) const;
	bool				read(const char* url, const void* object) const;
	void				set(const void* p, int value) const;
	bool				write(const char* url, void* object) const;
};
struct bsval {
	void*				data;
	const bsreq*		type;
	constexpr bsval() : data(0), type(0) {}
	constexpr bsval(void* data, const bsreq* type) : data(data), type(type) {}
	constexpr explicit operator bool() const { return data != 0; }
	bsval				dereference() const;
	int					get() const { return type->get(type->ptr(data)); }
	const char*			getname() const;
	void*				getptr() const { return type->ptr(data); }
	bsval				ptr(const char* url) const;
	void				set(int value) const { type->set(type->ptr(data), value); }
};