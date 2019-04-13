#include "xface/io.h"
#include "main.h"

static timestamp* read_stamp(io::stream& e) {
	timestamp ts; e.read(ts);
	auto& db = databases[ts.type];
	auto p = ts.getreference();
	if(!p) {
		p = (timestamp*)databases[ts.type].add();
		memcpy(p, &ts, sizeof(ts));
	}
	return p;
}

static void write_stamp(io::stream& e, const timestamp* v) {
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
	void set(coreobject*& value) {
		if(writemode) {
			if(value)
				write_stamp(source, value);
			else {
				timestamp ts;
				set(&ts, sizeof(ts));
			}
		} else
			value = (coreobject*)read_stamp(source);
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
		for(auto p = db.elements; p; p = p->next) {
			auto pe = p->begin() + db.size * p->count;
			for(auto pp = p->begin(); pp < pe; pp += db.size) {
				write_stamp(file, (timestamp*)pp);
			}
		}
	}
	return true;
}