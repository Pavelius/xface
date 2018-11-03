#pragma once

namespace compiler {
void*				rmreserve(void* data, unsigned count, unsigned& count_maximum, unsigned size);
template<class T> struct aref {
	T*				data;
	unsigned		count;
	constexpr aref() = default;
	template<unsigned N> constexpr aref(T(&data)[N]) : data(data), count(N) {}
	explicit operator bool() const { return count != 0; }
	T*				add() { return &data[count++]; }
	void			add(const T& e) { data[count++] = e; }
	constexpr T*	begin() { return data; }
	constexpr const T*	begin() const { return data; }
	void			clear() { count = 0; }
	constexpr T*	end() { return data + count; }
	constexpr const T* end() const { return data + count; }
	unsigned		indexof(const T* t) const { if(t<data || t>data + count) return 0xFFFFFFFF; return t - data; }
};
template<class T> struct amem : aref<T> {
	unsigned		count_maximum;
	constexpr amem() = default;
	T*				add() { this->data = (T*)rmreserve(this->data, this->count, count_maximum, sizeof(T)); return &this->data[this->count++]; }
	void			add(const T& e) { *add() = e; }
};
struct array {
	void*			data;
	unsigned		count, count_maximum, size;
	constexpr array(unsigned size = 0) : count(0), count_maximum(0), data(0), size(size) {}
	~array();
	void*			add();
	void			reserve(unsigned value) { data = rmreserve(data, count, count_maximum, size); }
};
struct string : amem<char> {
	amem<unsigned>	index;
	constexpr string() : amem(), index() {}
	unsigned		add(const char* value);
	const char*		get(unsigned value) const;
};
struct requisit {
	unsigned		id;
	requisit*		parent;
	requisit*		type;
	unsigned		count;
	unsigned		size;
	unsigned		offset;
	void* operator new(unsigned size);
	void operator delete(void* ptr, unsigned size);
	requisit*		add(const char* id, requisit* type);
	static requisit* add(const char* id);
	void			clear();
	requisit*		dereference() const;
	bool			isobject() const;
	requisit*		reference() const;
};
extern requisit		number[];
extern requisit		object[];
extern requisit		pointer[];
extern requisit		text[];
struct manager : string {
	amem<requisit>	requisits;
	requisit*		create(const char* id);
};
}