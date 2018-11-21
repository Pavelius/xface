#include "collection.h"
#include "crt.h"
#include "io.h"

#pragma once

// Fast and simple driver for streaming binary data
struct archive {

	struct dataset {
		void*			data;
		template<class T, unsigned N> constexpr dataset(T(&data)[N]) : data(&data), size(sizeof(T)), count(current_count), current_count(N), maximum_count(N) {}
		template<class T, unsigned N> constexpr dataset(adat<T, N>& data) : data(&data.data), size(sizeof(T)), count(data.count), current_count(0), maximum_count(N) {}
		void*			get(int index) const { return (char*)data + index * size; }
		int				indexof(void* p) const { if(((char*)p) >= ((char*)data) && ((char*)p) < ((char*)data + size * count)) return ((char*)p - (char*)data) / size; return -1; }
	private:
		unsigned		size;
		unsigned		maximum_count;
		unsigned		current_count;
		unsigned&		count;
	};
	io::stream&			source;
	bool				writemode;
	aref<dataset>		pointers;

	constexpr archive(io::stream& source, bool writemode) : source(source), writemode(writemode), pointers() {}
	constexpr archive(io::stream& source, bool writemode, const aref<dataset>& pointers) : source(source), writemode(writemode), pointers(pointers) {}

	virtual void		set(void* value, unsigned size);
	virtual void		setpointer(void** value);
	virtual void		setstring(const char*& value);
	bool				signature(const char* id);
	bool				version(short major, short minor);

	// Any pointer class
	template<class T> void set(T*& value) {
		setpointer((void**)&value);
	}
	// Strings case
	template<> void set<const char>(const char*& e) {
		setstring(e);
	}
	// Array with fixed count
	template<typename T, unsigned N> void set(T(&value)[N]) {
		for(int i = 0; i < N; i++)
			set(value[i]);
	};
	// Fixed data collection
	template<typename T, unsigned N> void set(adat<T, N>& value) {
		set(value.count);
		for(auto& e : value)
			set(e);
	}
	// Fixed amem collection
	template<typename T> void set(arem<T>& value) {
		set(value.count);
		if(!writemode)
			value.reserve(value.count);
		for(auto& e : value)
			set(e);
	}
	// Custom aref collection
	template<typename T> void set(aref<T>& value) {
		set(value.count);
		set(value.data);
	}
	// All simple types and requisites
	template<class T> void set(T& value) {
		set(&value, sizeof(value));
	}
};
