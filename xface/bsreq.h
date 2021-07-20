#include "crt.h"

#pragma once

#define	BSREQ(fn) {#fn, FO(data_type, fn),\
sizeof(meta_size<decltype(data_type::fn)>::value),\
sizeof(data_type::fn),\
meta_count<decltype(data_type::fn)>::value,\
bsmeta<meta_decoy<decltype(data_type::fn)>::value>::meta,\
meta_kind<decltype(data_type::fn)>::value,\
bsdata<meta_decoy<decltype(data_type::fn)>::value>::source_ptr}
#define BSMETA(e) template<> const bsreq bsmeta<e>::meta[]

// Metadata field descriptor
struct bsreq {
	// Basic metadata types
	enum class kind : unsigned char {
		None, Number, Text, Scalar, Enum, Reference,
		ADat, List, ARem, CFlags
	};
	const char*				id; // field identifier
	unsigned				offset; // offset from begin of class or object
	unsigned				size; // size of single element
	unsigned				lenght; // total size in bytes of all field (array has size*count)
	unsigned				count; // count of elements
	const bsreq*			type; // metadata of element
	kind					subtype; // metadata subtype
	array*					source; // data source for enumerators
	constexpr explicit operator bool() const { return id != 0; }
	void*					dereference(const void* data, const bsreq** result) const;
	bool					equal(const void* v1, const void* v2) const;
	const bsreq*			find(const char* name) const;
	const bsreq*			find(const char* name, unsigned count) const;
	const bsreq*			find(const char* name, const bsreq* type) const;
	int						get(const void* p) const;
	const char*				get(const void* p, char* result, const char* result_max) const;
	const char*				gets(const void* p) const;
	const bsreq*			getname() const;
	const char*				getmetaname() const; // Extern function
	bool					is(kind v) const { return subtype == v; }
	bool					issimple() const { return is(kind::Number) || is(kind::Text); }
	bool					match(const void* p, const char* name) const;
	char*					ptr(const void* data) const { return (char*)data + offset; }
	char*					ptr(const void* data, int index) const { return (char*)data + offset + index * size; }
	void*					ptr(const void* data, const char* url, const bsreq** result) const;
	bool					read(const char* url, const void* object) const;
	void					set(const void* p, int value) const;
	bool					write(const char* url, void* object) const;
};
NOBSDATA(bsreq)
// Abstract data source descriptor
struct bsinf {
	const char*				id;
	array*					source;
	const bsreq*			type;
	constexpr explicit operator bool() const { return id != 0; }
};
struct bsparse {
	enum class errors {
		None,
		NotFoundIdentifier1p, Expected1p, File1pNotFound,
		NotFoundType, NotFoundBase1p, NotFoundMember1pInBase2p,
		ExpectedIdentifier,
	};
	const bsinf*			metadata;
	virtual void			error(errors id, const char* url, int line, int column, const char* format_param) {}
	virtual const char*		getinclude(char* result, const char* result_end, const char* name) { return 0; }
	virtual const bsreq*	getmeta(const char* name);
	virtual const bsreq*	getrequisit(const bsreq* type, const char* name) { return type->find(name); }
	virtual array*			getsource(const bsreq* type);
	void					read(const char* url);
	bool					read(const char* url, const char* id, const char** requisits);
};
// Abstract metadata class
template<typename T> struct bsmeta {
	typedef T				data_type;
	static const bsreq		meta[];
};
template<> struct bsmeta<unsigned char> : bsmeta<int> {};
template<> struct bsmeta<char> : bsmeta<int> {};
template<> struct bsmeta<unsigned short> : bsmeta<int> {};
template<> struct bsmeta<short> : bsmeta<int> {};
template<> struct bsmeta<unsigned> : bsmeta<int> {};
template<> struct bsmeta<bool> : bsmeta<int> {};
// Get object presentation
template<class T> const char* getstr(const T e) { return bsdata<T>::elements[e].name; }
// Untility structures
template<typename T, T v> struct static_value { static constexpr T value = v; };
template<int v> struct static_int : static_value<int, v> {};
// Get array elments
template<class T> struct meta_count : static_int<1> {};
template<class T, unsigned N> struct meta_count<T[N]> : static_int<N> {};
template<class T> struct meta_count<T[]> : static_int<0> {};
template<class T, unsigned N> struct meta_count<adat<T, N>> : static_int<N> {};
// Get base size
template<class T> struct meta_size : meta_decoy<T> {};
template<class T> struct meta_size<T*> { typedef T* value; };
template<class T> struct meta_size<const T*> { typedef T* value; };
template<class T, unsigned N> struct meta_size<T[N]> : meta_size<T> {};
template<class T> struct meta_size<T[]> : meta_size<T> {};
// Get kind
template<class T> struct meta_kind : static_value<bsreq::kind, __is_enum(T) ? bsreq::kind::Enum : bsreq::kind::Scalar> {};
template<> struct meta_kind<const char*> : static_value<bsreq::kind, bsreq::kind::Text> {};
template<> struct meta_kind<char> : static_value<bsreq::kind, bsreq::kind::Number> {};
template<> struct meta_kind<short> : static_value<bsreq::kind, bsreq::kind::Number> {};
template<> struct meta_kind<int> : static_value<bsreq::kind, bsreq::kind::Number> {};
template<> struct meta_kind<unsigned char> : static_value<bsreq::kind, bsreq::kind::Number> {};
template<> struct meta_kind<unsigned short> : static_value<bsreq::kind, bsreq::kind::Number> {};
template<> struct meta_kind<unsigned int> : static_value<bsreq::kind, bsreq::kind::Number> {};
template<> struct meta_kind<bool> : static_value<bsreq::kind, bsreq::kind::Number> {};
template<class T> struct meta_kind<T*> : static_value<bsreq::kind, bsreq::kind::Reference> {};
template<class T> struct meta_kind<T**> {}; // Not allowed complex pointer
template<class T> struct meta_kind<const T*> : static_value<bsreq::kind, bsreq::kind::Reference> {};
template<class T> struct meta_kind<const T> : static_value<bsreq::kind, meta_kind<T>::value> {};
template<class T, unsigned N> struct meta_kind<T[N]> : static_value<bsreq::kind, meta_kind<T>::value> {};
template<class T, unsigned N> struct meta_kind<adat<T, N>> : static_value<bsreq::kind, bsreq::kind::ADat> {};
template<class T> struct meta_kind<std::initializer_list<T>> : static_value<bsreq::kind, bsreq::kind::List> {};
template<class T, class DT> struct meta_kind<cflags<T, DT>> : static_value<bsreq::kind, bsreq::kind::CFlags> {};