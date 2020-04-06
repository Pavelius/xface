#include "crt.h"
#include "bsreq.h"
#include "io.h"

namespace {
struct context {
	io::stream&		file;
	bool			writemode;
	arem<void*>		references;
	void serial(void* v, unsigned lenght) {
		if(writemode)
			file.write(v, lenght);
		else
			file.read(v, lenght);
	}
	void serial(void** pv, const bsreq& e) {
		unsigned i = 0;
		auto p = *pv;
		if(writemode) {
			if(p)
				i = references.indexof(p);
			file.write(&i, sizeof(i));
			if(i != 0xFFFFFFFF)
				return;
			if(e.is(KindText)) {
				i = zlen((const char*)p);
				file.write(&i, sizeof(i));
				file.write(p, i);
			} else
				serial(p, e.type, e.type + 1);
		} else {
			char buffer[2048];
			file.read(&i, sizeof(i));
			if(!i)
				*pv = 0;
			else if(i != 0xFFFFFFFF)
				*pv = references[i];
			else {
				auto pb = buffer;
				if(e.is(KindText)) {
					file.read(&i, sizeof(i));
					if(i > sizeof(buffer) / sizeof(buffer[0]) - 1)
						pb = new char[i + 1];
					file.read(pb, i);
					pb[i] = 0;
					auto ppb = (void*)szdup(pb);
					*pv = ppb;
					references.add(ppb);
				} else {
					auto pid = references.getcount();
					references.add();
					serial(pb, e.type, e.type + 1);
					// TODO: find element
					references[pid] = 0;
				}
				if(pb != buffer)
					delete pb;
			}
		}
	}
	void serial(const void* object, const bsreq* records, const bsreq* records_stop = 0) {
		for(auto p = records; *p; p++) {
			if(p == records_stop)
				break;
			switch(p->subtype) {
			case KindNumber:
			case KindCFlags:
			case KindEnum:
				serial(p->ptr(object), p->lenght);
				break;
			case KindText:
			case KindReference:
				for(unsigned i = 0; i < p->count; i++)
					serial((void**)p->ptr(object, i), *p->type);
				break;
			case KindScalar:
				for(unsigned i = 0; i < p->count; i++)
					serial(p->ptr(object, i), p->type);
				break;
			}
		}
	}
	void read_object(array* pb, const bsreq* meta) {
	}
	void write_object(const void* pv, const bsreq* meta) {
	}
	constexpr context(io::stream& file, bool writemode) : file(file), writemode(writemode) {}
};
}

bool bsreq::write(const char* url, void* object) const {
	io::file file(url, StreamWrite);
	if(!file)
		return false;
	context e(file, true);
	e.write_object(object, this);
	return true;
}

bool bsreq::read(const char* url, const void* object) const {
	io::file file(url, StreamRead);
	if(!file)
		return false;
	context e(file, false);
	e.serial(object, this);
	return true;
}