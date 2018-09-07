#pragma once

#define FO(c,f) (const int)&((c*)0)->f
#define	BSREQ(cls, field, type) {#field, FO(cls,field),\
bsreq::info<decltype(cls::field)>::size,\
sizeof(cls::field),\
bsreq::info<decltype(cls::field)>::count,\
type,\
bsreq::refi<decltype(cls::field)>::count,\
bsreq::enmi<decltype(cls::field)>::value}

const int bsreq_max_text = 8192;

// Metadata field descriptor
struct bsreq {
	template<class T> struct refi { static constexpr int count = 0; };
	template<class T> struct refi<T*> { static constexpr int count = 1 + refi<T>::count; };
	template<class T, int N> struct refi<T[N]> { static constexpr int count = refi<T>::count; };
	template<class T> struct refi<T[]> { static constexpr int count = refi<T>::count; };
	template<class T> struct enmi { static constexpr bool value = __is_enum(T); };
	template<class T> struct enmi<T*> { static constexpr bool value = __is_enum(T); };
	template<class T, int N> struct enmi<T[N]> { static constexpr bool value = __is_enum(T); };
	template<class T> struct info {
		static constexpr int size = sizeof(T);
		static constexpr int count = 1;
	};
	template<class T, unsigned N> struct info<T[N]> {
		static constexpr int size = sizeof(T);
		static constexpr int count = N;
	};
	template<class T> struct info<T[]> {
		static constexpr int size = sizeof(T);
		static constexpr int count = 0;
	};
	const char*			id; // field identifier
	unsigned			offset; // offset from begin of class or object
	unsigned			size; // size of single element
	unsigned			lenght; // total size in bytes of all field (array has size*count)
	unsigned			count; // count of elements
	const bsreq*		type; // metadata of element
	unsigned char		reference; // 1+ is reference
	unsigned char		isenum;
	//
	operator bool() const { return id != 0; }
	//
	const bsreq*		find(const char* name) const;
	const bsreq*		find(const char* name, const bsreq* type) const;
	int					get(const void* p) const;
	const char*			getdata(char* result, const char* id, const void* object, bool tobuffer) const;
	const bsreq*		getkey() const;
	bool				issimple() const { return type == 0; }
	bool				match(const void* p, const char* name) const;
	inline const char*	ptr(const void* data) const { return (const char*)data + offset; }
	inline const char*	ptr(const void* data, int index) const { return (const char*)data + offset + index*size; }
	void				set(const void* p, int value) const;
	void				setdata(const char* result, const char* id, void* object) const;
};
struct bsval {
	const bsreq*		type;
	void*				data;
	operator bool() const { return data != 0; }
	int					get() const { return type->get(type->ptr(data)); }
	void				set(int value) { type->set(type->ptr(data), value); }
};
struct bsfunc {
	const char*			id;
	const char*			parameters[8];
	const char*			code;
	operator bool() const { return id != 0; }
};
extern bsreq			number_type[]; // standart integer value
extern bsreq			text_type[]; // stantart zero ending string
extern bsreq			bsreq_type[]; // requisit metadata