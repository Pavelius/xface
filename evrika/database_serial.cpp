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