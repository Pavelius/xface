#include "initializer_list.h"

#pragma once

unsigned					rmoptimal(unsigned need_count);
void*						rmreserve(void* data, unsigned new_size);
void*						rmreserve(void* data, unsigned count, unsigned& count_maximum, unsigned size);

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
	void*					end(unsigned size) { return (char*)this + sizeof(*this) + count*size; }
	unsigned				getcount() const;
	void*					get(int index, unsigned size) const;
	int						indexof(const void* e, unsigned size) const;
	void					release();
};
// Untility structures
template<typename T, T v> struct static_value { static constexpr T value = v; };
template<int v> struct static_int : static_value<int, v> {};
// Storge like vector
template<class T, int count_max = 128>
struct adat {
	T						data[count_max];
	unsigned				count;
	constexpr adat() : count(0) {}
	constexpr adat(std::initializer_list<T> list) : count(0) { for(auto& e : list) *add() = e; }
	constexpr const T& operator[](unsigned index) const { return data[index]; }
	constexpr T& operator[](unsigned index) { return data[index]; }
	explicit operator bool() const { return count != 0; }
	T*						add() { if(count < count_max) return data + (count++); return 0; }
	void					add(const T& e) { if(count < count_max) data[count++] = e; }
	T*						begin() { return data; }
	const T*				begin() const { return data; }
	void					clear() { count = 0; }
	T*						end() { return data + count; }
	const T*				end() const { return data + count; }
	const T*				endof() const { return data + count_max; }
	int						getcount() const { return count; }
	int						getmaximum() const { return count_max; }
	int						indexof(const T* e) const { if(e >= data && e < data + count) return e - data; return -1; }
	int						indexof(const T t) const { for(unsigned i = 0; i < count; i++) if(data[i] == t) return i; return -1; }
	bool					is(const T t) const { return indexof(t) != -1; }
	void					remove(int index, int remove_count = 1) { if(index < 0) return; if(index<int(count - 1)) memcpy(data + index, data + index + 1, sizeof(data[0])*(count - index - 1)); count--; }
};
template<class T, unsigned N = 64>
class agrw : public arraydata {
	template<class T>
	struct iterator : arraydata::iterator {
		iterator(const arraydata* source) { initialize(source, sizeof(T)); }
		constexpr T& operator*() const { return *((T*)pb); }
		constexpr bool operator!=(const iterator& e) const { return pb != e.pb; }
		void operator++() { increment(sizeof(T)); }
	};
	T						data[N]; // Размер data[] увеличивается динамически
public:
	typedef T				element;
	constexpr agrw() : arraydata(N), data() {}
	~agrw() { release(); }
	T& operator[](int index) { return *((T*)arraydata::get(index, sizeof(T))); }
	const T& operator[](int index) const { return *((const T*)arraydata::get(index, sizeof(T))); }
	T*						add() { auto p = (T*)arraydata::add(sizeof(T)); *p = T(); return p; }
	iterator<T>				begin() const { return iterator<T>(this); }
	iterator<T>				end() const { return iterator<T>(0); }
	T*						get(int index) const { return (T*)arraydata::get(index, sizeof(T)); }
	int						indexof(const T* e) const { return arraydata::indexof(e, sizeof(T)); }
};
// Reference to array with dymanic size
template<class T>
struct aref {
	T*						data;
	unsigned				count;
	constexpr aref() = default;
	constexpr aref(T* source, unsigned count) : data(source), count(count) {}
	template<unsigned N> constexpr aref(T(&data)[N]) : data(data), count(N) {}
	template<unsigned N> constexpr aref(adat<T, N>& source) : data(source.data), count(source.count) {}
	constexpr T& operator[](int index) { return data[index]; }
	constexpr const T& operator[](int index) const { return data[index]; }
	explicit operator bool() const { return count != 0; }
	constexpr T*			begin() { return data; }
	constexpr const T*		begin() const { return data; }
	constexpr T*			end() { return data + count; }
	constexpr const T*		end() const { return data + count; }
	int						getcount() const { return count; }
	int						indexof(const T* t) const { if(t<data || t>data + count) return -1; return t - data; }
	int						indexof(const T t) const { for(unsigned i = 0; i < count; i++) if(data[i] == t) return i; return -1; }
	bool					is(const T t) const { return indexof(t) != -1; }
};
// Autogrow typized array
template<class T>
struct arem : aref<T> {
	unsigned				count_maximum;
	constexpr arem() : aref<T>(0, 0), count_maximum(0) {}
	constexpr arem(T* source, unsigned count) : aref<T>(source, count), count_maximum(0) {}
	~arem() { if(aref<T>::data && count_maximum) delete aref<T>::data; }
	T*						add() { reserve(this->count + 1); return &aref<T>::data[aref<T>::count++]; }
	void					add(const T& e) { *(add()) = e; }
	void					clear() { aref<T>::count = 0; }
	void					remove(int index, int elements_count = 1) { if(index < 0 || index >= count) return; count -= elements_count; if(index >= count) return; memmove(data + index, data + index + elements_count, sizeof(data[0])*(count - index)); }
	void					reserve(unsigned count) { aref<T>::data = (T*)rmreserve(aref<T>::data, count, count_maximum, sizeof(T)); }
};
// Abstract flag data bazed on enumerator
template<typename T, typename DT = unsigned>
struct cflags {
	DT						data;
	constexpr cflags() : data(0) {}
	constexpr cflags(std::initializer_list<T> list) : data() { for(auto e : list) add(e); }
	constexpr void			add(const T id) { data |= 1 << id; }
	constexpr void			clear() { data = 0; }
	constexpr bool			is(const T id) const { return (data & (1 << id)) != 0; }
	constexpr void			remove(T id) { data &= ~(1 << id); }
};
// Abstract pair element
template<typename K, typename V>
struct pair {
	K						key;
	V						value;
};
// Abstract map collection
template<typename K, typename V>
struct amap : arem<pair<K, V>> {
	void					add(const K& key, const V& value) { auto p = const_cast<pair<K, V>*>(find(key)); if(!p) { p = arem<pair<K, V>>::add(); p->key = key; } p->value = value; }
	const pair<K, V>*		find(K key) const { for(auto& e : *this) if(e.key == key) return &e; return 0; }
	const pair<K, V>*		findv(V value) const { for(auto& e : *this) if(e.value == value) return &e; return 0; }
	V						get(K key) const { auto p = find(key); if(p) return p->value; return V(); }
	K						getv(V value) const { auto p = findv(value); if(p) return p->key; return K(); }
	bool					is(const K& key) const { return find(key) != 0; }
};
template<typename T = char>
class iterator {
	char*					current;
	unsigned				size;
public:
	constexpr iterator(char* current, unsigned size) : current(current), size(size) {}
	constexpr T* operator*() const { return (T*)current; }
	constexpr bool operator!=(const iterator& e) const { return e.current != current; }
	constexpr void operator++() { current += size; }
};
// Abstract array vector
struct array {
	void*					data;
	unsigned				size;
	unsigned&				count;
	unsigned				count_value;
	unsigned				count_maximum;
	bool					can_grow;
	constexpr array() : data(0), size(0), count_maximum(0), count(count_value), count_value(0), can_grow(false) {}
	constexpr array(unsigned size) : data(0), size(size), count_maximum(0), count(count_value), count_value(0), can_grow(true) {}
	constexpr array(void* data, unsigned size, unsigned count_maximum) : data(data), size(size), count_maximum(count_maximum), count(count_value), count_value(count_maximum), can_grow(false) {}
	constexpr array(void* data, unsigned size, unsigned count_maximum, unsigned& count) : data(data), size(size), count_maximum(count_maximum), count(count), count_value(0), can_grow(false) {}
	template<typename T, unsigned N> constexpr array(adat<T, N>& e) : array(e.data, sizeof(T), N, e.count) {}
	template<typename T> constexpr array(const aref<T>& e) : array(e.data, sizeof(T), e.count) {}
	template<typename T, unsigned N> constexpr array(T(&e)[N]) : array(e, sizeof(T), N) {}
	template<typename T> constexpr array(T(&e)[]) : array(e, sizeof(T), 1) {}
	~array();
	void*					add();
	void*					add(const void* element);
	char*					begin() { return (char*)data; }
	const char*				begin() const { return (char*)data; }
	void					clear();
	char*					end() { return (char*)data + size * count; }
	const char*				end() const { return (char*)data + size * count; }
	int						find(const char* value, unsigned offset) const;
	void*					get(int index) const { return (char*)data + size * index; }
	unsigned				getmaxcount() const { return count_maximum; }
	unsigned				getcount() const { return count; }
	unsigned				getsize() const { return size; }
	int						indexof(const void* element) const;
	void*					insert(int index, const void* element);
	bool					isgrowable() const { return can_grow; }
	void					remove(int index, int elements_count);
	void					setcount(unsigned value) { count = value; }
	void					setup(unsigned size);
	void					shift(int i1, int i2, unsigned c1, unsigned c2);
	void					sort(int i1, int i2, int(*compare)(const void* p1, const void* p2, void* param), void* param);
	void					swap(int i1, int i2);
	void					reserve(unsigned count);
};
struct arrayref {
	char**					data;
	unsigned&				count;
	unsigned				size;
	template<typename T> constexpr arrayref(aref<T>& e) : data((char**)&e.data), count(e.count), size(sizeof(T)), count_value(), data_value() {}
	template<typename T> constexpr arrayref(T(&e)[]) : data(&data_value), count(count_value), size(sizeof(T)), count_value(1), data_value((char*)&e) {}
	template<typename T, unsigned N> constexpr arrayref(adat<T, N>& e) : data((char**)&e.data), count(e.count), size(sizeof(T)), count_value(), data_value() {}
	template<typename T, unsigned N> constexpr arrayref(T(&e)[N]) : data(&data_value), count(count_value), size(sizeof(T)), count_value(), data_value((char*)&e) {}
	void*					get(int index) const { return (char*)(*data) + size * index; }
	int						indexof(const void* t) const { if(t<(*data) || t>(*data) + count*size) return -1; return ((char*)t - (*data))/size; }
private:
	unsigned				count_value;
	char*					data_value;
};