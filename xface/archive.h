#include "crt.h"
#include "io.h"

#pragma once

// Fast and simple driver for streaming binary data
struct archive {
	typedef std::initializer_list<void*> arrayref;
	io::stream&			source;
	bool				writemode;
	std::initializer_list<arrayref> pointers;
	constexpr archive(io::stream& source, bool writemode) : source(source), writemode(writemode), pointers() {}
	constexpr archive(io::stream& source, bool writemode, const std::initializer_list<arrayref>& pointers) : source(source), writemode(writemode), pointers(pointers) {}
	virtual void		set(void* value, unsigned size);
	virtual void		setpointer(void** value);
	virtual void		setstring(const char** value);
	bool				signature(const char* id);
	bool				version(short major, short minor);
	// Any pointer class
	template<class T> void set(T*& value) {
		setpointer((void**)&value);
	}
    void set(const char*& e) {
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
	template<typename T> void set(vector<T>& value) {
		set(value.count);
		if(!writemode)
			value.reserve(value.count);
		for(auto& e : value)
			set(e);
	}
	// All simple types and requisites
	template<class T> void set(T& value) {
		set(&value, sizeof(value));
	}
};