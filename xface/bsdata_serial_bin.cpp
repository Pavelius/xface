#include "archive.h"
#include "bsdata.h"

template<> void archive::set<bsreq>(bsreq& e) {
	set(e.id);
	set(e.offset);
	set(e.size);
	set(e.lenght);
	set(e.count);
	set(e.type);
	set(e.reference);
	set(e.subtype);
}

void bsdata::write(const char* url, const array& source, const bsreq* fields) {
	io::file file(url, StreamWrite);
	if(!file)
		return;
	archive a(file, true);
	if(!a.signature("MTD"))
		return;
	if(!a.version(1, 0))
		return;
}