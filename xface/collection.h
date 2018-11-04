#include "initializer_list.h"

#pragma once

unsigned					rmoptimal(unsigned need_count);
void*						rmreserve(void* data, unsigned new_size);

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
	int						getcount() const { return count; }
	int						indexof(const T* e) const { if(e >= data && e < data + count) return e - data; return -1; }
	int						indexof(const T t) const { for(unsigned i = 0; i < count; i++) if(data[i] == t) return i; return -1; }
	bool					is(const T t) const { return indexof(t) != -1; }
	void					remove(int index, int remove_count = 1) { if(index < 0) return; if(index<int(count - 1)) memcpy(data + index, data + index + 1, sizeof(data[0])*(count - index - 1)); count--; }
};
// Reference to array with dymanic size
template<class T> struct aref {
	T*						data;
	unsigned				count;
	constexpr aref() = default;
	template<unsigned N> constexpr aref(T(&data)[N]) : data(data), count(N) {}
	template<unsigned N> constexpr aref(adat<T, N>& source) : data(source.data), count(source.count) {}
	constexpr T& operator[](int index) { return data[index]; }
	constexpr const T& operator[](int index) const { return data[index]; }
	explicit operator bool() const { return count != 0; }
	T*						add() { return &data[count++]; }
	void					add(const T& e) { data[count++] = e; }
	constexpr T*			begin() { return data; }
	constexpr const T*		begin() const { return data; }
	void					clear() { count = 0; }
	constexpr T*			end() { return data + count; }
	constexpr const T*		end() const { return data + count; }
	int						indexof(const T* t) const { if(t<data || t>data + count) return -1; return t - data; }
	int						indexof(const T t) const { for(unsigned i = 0; i < count; i++) if(data[i] == t) return i; return -1; }
	bool					is(const T t) const { return indexof(t) != -1; }
	void					remove(int index, int elements_count = 1) { if(index < 0 || index >= count) return; count -= elements_count; if(index >= count) return; memmove(data + index, data + index + elements_count, sizeof(data[0])*(count - index)); }
};
// Autogrow typized array
template<class T> struct arem : aref<T> {
	unsigned				count_maximum;
	constexpr arem() : aref<T>(), count_maximum() {}
	T*						add() { reserve(this->count + 1); return &aref<T>::data[aref<T>::count++];}
	void					add(const T& e) { *(add()) = e; }
	void					reserve(unsigned count) { if(count >= count_maximum) { count_maximum = rmoptimal(count + 1); this->data = (T*)rmreserve(this->data, count_maximum * sizeof(T)); } }
};
// Abstract flag data bazed on enumerator
template<typename T, typename DT = unsigned> struct cflags {
	DT						data;
	constexpr cflags() : data(0) {}
	constexpr cflags(std::initializer_list<T> list) : data() { for(auto e : list) add(e); }
	constexpr void			add(const T id) { data |= 1 << id; }
	constexpr void			clear() { data = 0; }
	constexpr bool			is(const T id) const { return (data & (1 << id)) != 0; }
	constexpr void			remove(T id) { data &= ~(1 << id); }
};
// Abstract array vector
class array {
	void*					data;
	unsigned				size;
	unsigned&				count;
	unsigned				count_value;
	unsigned				count_maximum;
	bool					can_grow;
public:
	constexpr array() : data(0), size(0), count_maximum(0), count(count_value), count_value(0), can_grow(false) {}
	constexpr array(unsigned size) : data(0), size(size), count_maximum(0), count(count_value), count_value(0), can_grow(true) {}
	constexpr array(void* data, unsigned size, unsigned count_maximum) : data(data), size(size), count_maximum(count_maximum), count(count_value), count_value(count_maximum), can_grow(false) {}
	constexpr array(void* data, unsigned size, unsigned count_maximum, unsigned& count) : data(data), size(size), count_maximum(count_maximum), count(count), count_value(0), can_grow(false) {}
	template<typename T, unsigned N> constexpr array(adat<T, N>& e) : array(e.data, sizeof(T), N, e.count) {}
	template<typename T> constexpr array(const aref<T>& e) : array(e.data, sizeof(T), e.count) {}
	template<typename T, unsigned N> constexpr array(T(&e)[N]) : array(e.data, sizeof(T), N) {}
	~array();
	void*					add();
	void					add(const void* element);
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
	void					sort(int i1, int i2, int(*compare)(const void* p1, const void* p2, void* param), void* param);
	void					swap(int i1, int i2);
	void					reserve(unsigned count);
};