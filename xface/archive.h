#include "collection.h"
#include "crt.h"
#include "io.h"

#pragma once

// Fast and simple driver for streaming binary data
struct archive {
	io::stream&			source;
	bool				writemode;
	aref<arrayref>		pointers;
	constexpr archive(io::stream& source, bool writemode) : source(source), writemode(writemode), pointers() {}
	constexpr archive(io::stream& source, bool writemode, const aref<arrayref>& pointers) : source(source), writemode(writemode), pointers(pointers) {}
	virtual void		set(void* value, unsigned size);
	virtual void		setpointer(void** value);
	virtual void		setstring(const char** value);
	bool				signature(const char* id);
	bool				version(short major, short minor);

	// Any pointer class
	template<class T> void set(T*& value) {
		setpointer((void**)&value);
	}
	// Strings case
	template<> void set<const char>(const char*& e) {
		setstring(&e);
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
