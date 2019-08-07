#include "xface/io.h"
#include "main.h"

static stampi* read_stamp(io::stream& e) {
	stampi ts; e.read(ts);
	auto& db = ts.getbase();
	auto p = ts.getreference();
	if(!p) {
		p = (stampi*)db.add();
		memcpy(p, &ts, sizeof(ts));
	}
	return p;
}

static void write_stamp(io::stream& e, const stampi* v) {
	e.write(v, sizeof(*v));
}

// Fast and simple driver for streaming binary data
struct archive {
	io::stream&	source;
	bool		writemode;
	
	constexpr archive(io::stream& source, bool writemode) : source(source), writemode(writemode) {}
	
	void set(void* value, unsigned size) {
		if(writemode)
			source.write(value, size);
		else
			source.read(value, size);
	}
	
	// Any pointer class
	void set(objecti*& value) {
		if(writemode) {
			if(value)
				write_stamp(source, value);
			else {
				stampi ts;
				set(&ts, sizeof(ts));
			}
		} else
			value = (objecti*)read_stamp(source);
	}

	// Strings case
	void set(const char*& e) {
		unsigned len = 0;
		char temp[128 * 128];
		if(writemode) {
			if(*e)
				len = zlen(e);
			source.write(&len, sizeof(len));
			if(len)
				source.write(e, len);
		} else {
			source.read(&len, sizeof(len));
			e = 0;
			if(len) {
				source.read(temp, len);
				temp[len] = 0;
				e = szdup(temp);
			}
		}
	}

	// Array with fixed count
	template<typename T, unsigned N> void set(T(&value)[N]) {
		for(int i = 0; i < N; i++)
			set(value[i]);
	};

	// All simple types and requisites
	template<class T> void set(T& value) {
		set(&value, sizeof(value));
	}

};

bool database::readfile(const char* url) {
	io::file file(url, StreamRead);
	if(!file)
		return false;
	while(true) {
		auto p = read_stamp(file);
		if(!p)
			return true;
	}
}

bool database::writefile(const char* url) {
	io::file file(url, StreamWrite);
	if(!file)
		return false;
	for(auto& db : databases) {
		for(auto pp : db)
			write_stamp(file, (stampi*)pp);
	}
	return true;
}