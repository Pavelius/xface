#include "initializer_list.h"

#pragma once

unsigned					rmoptimal(unsigned need_count);
void*						rmreserve(void* data, unsigned new_size);

// Abstract collection
struct collection {
	virtual void*			add() = 0; // Add new empthy element to collection
	void					add(const void* element); // Add new element to collection
	virtual void			clear() = 0; // Clear all elements in collection
	int						find(const char* value, unsigned offset = 0); // Find value by name
	virtual void*			get(int index) const = 0; // Get content of element with index
	virtual unsigned		getmaxcount() const = 0; // Get maximum possible elements
	virtual unsigned		getcount() const = 0; // Get count of elements in collection
	virtual unsigned		getsize() const = 0; // Get size of one element in collection
	virtual int				indexof(const void* element) const;
	void*					insert(int index, const void* object); // Insert new element to collection by specific index
	bool					read(const char* url, const struct bsreq* fields);
	virtual void			remove(int index, int count = 1); // Remove element from collection
	virtual void			setcount(unsigned value) = 0;
	void					sort(int i1, int i2, int(*compare)(const void* p1, const void* p2, void* param), void* param);	// Sort collection
	void					swap(int i1, int i2); // Swap elements
	bool					write(const char* url, const struct bsreq* fields);
};
// Storge like vector
template<class T, int count_max = 128>
struct adat {
	T						data[count_max];
	unsigned				count;
	//
	constexpr adat() : data(), count(0) {}
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
	T*						endof() { return data + count_max; }
	const T*				end() const { return data + count; }
	template<class Z> T*	find(Z id) { auto e1 = data + count; for(T* e = data; e < e1; e++) { if(e->id == id) return e; } return 0; }
	inline int				getcount() const { return count; }
	int						indexof(const T* e) const { if(e >= data && e <= data + count) return e - data; return -1; }
	int						indexof(const T t) const { for(unsigned i = 0; i < count; i++) if(data[i] == t) return i; return -1; }
	bool					is(const T t) const { for(unsigned i = 0; i < count; i++) if(data[i] == t) return true; return false; }
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
	bool					is(const T value) const { return indexof(value) != -1; }
	void					remove(int index, int elements_count = 1) { if(index < 0 || index >= count) return; count -= elements_count; if(index >= count) return; memmove(data + index, data + index + elements_count, sizeof(data[0])*(count - index)); }
};
template<class T> struct arem : aref<T> {
	unsigned				count_maximum;
	constexpr arem() : aref<T>(), count_maximum() {}
	T* add() {
		if(aref<T>::count >= count_maximum) {
			count_maximum = rmoptimal(aref<T>::count + 1);
			aref<T>::data = (T*)rmreserve(aref<T>::data, count_maximum * sizeof(T));
		}
		return &aref<T>::data[aref<T>::count++];
	}
	void					add(const T& e) { *(add()) = e; }
};
// Abstract flag data bazed on enumerator
template<typename T, typename DT = unsigned> class cflags {
	static constexpr T maximum = (T)(sizeof(DT) * 8);
	struct iter {
		T	current;
		DT	data;
		iter(T current, DT data) : current(getnext(current, data)), data(data) {}
		T operator*() const { return (T)current; }
		bool operator!=(const iter& e) const { return e.current != current; }
		void operator++() { current = getnext((T)(current + 1), data); }
		constexpr T getnext(T current, unsigned data) const {
			while(current < maximum && (data & (1 << current)) == 0)
				current = (T)(current + 1);
			return current;
		}
	};
public:
	DT						data;
	constexpr cflags() : data(0) {}
	constexpr cflags(std::initializer_list<T> list) : data() { for(auto e : list) add(e); }
	constexpr void			add(T id) { data |= 1 << id; }
	iter					begin() const { return iter((T)0, data); }
	void					clear() { data = 0; }
	iter					end() const { return iter(maximum, data); }
	constexpr bool			is(T id) const { return (data & (1 << id)) != 0; }
	constexpr void			remove(T id) { data &= ~(1 << id); }
};
struct amem : collection {
	constexpr amem(unsigned size = 0) : data(0), size(size), count(0), count_maximum(0) {}
	~amem();
	virtual void*			add() override;
	virtual void			clear() override;
	virtual void*			get(int index) const override { return (char*)data + size * index; }
	virtual unsigned		getmaxcount() const override { return count_maximum; }
	virtual unsigned		getcount() const override { return count; }
	virtual unsigned		getsize() const override { return size; }
	void					reserve(unsigned new_count);
	virtual void			setcount(unsigned value) { count = value;  }
	void					setup(unsigned size);
private:
	void*					data;
	unsigned				size;
	unsigned				count;
	unsigned				count_maximum;
};
struct avec : collection {
	constexpr avec() : data(0), size(0), count_maximum(0), count(count_value), count_value(0) {}
	avec(const avec& source) = default;
	template<typename T, unsigned N> constexpr avec(adat<T, N>& e) : data(e.data), size(sizeof(T)), count(e.count), count_maximum(N), count_value(0) {}
	template<typename T> constexpr avec(aref<T>& e) : data(e.data), size(sizeof(T)), count(e.count), count_maximum(e.count), count_value(0) {}
	template<typename T, unsigned N> constexpr avec(T e[N]) : data(e), size(sizeof(T)), count(count_value), count_maximum(N), count_value(0) {}
	virtual void*			add() override { return (char*)data + getsize()*((count < count_maximum) ? count++ : 0); }
	virtual void			clear() override { count = 0; }
	virtual void*			get(int index) const override { return (char*)data + size * index; }
	virtual unsigned		getmaxcount() const override { return count_maximum; }
	virtual unsigned		getcount() const override { return count; }
	virtual unsigned		getsize() const override { return size; }
	virtual void			setcount(unsigned value) { count = value; }
private:
	void*					data;
	unsigned				size;
	unsigned&				count;
	unsigned				count_maximum;
	unsigned				count_value;
};