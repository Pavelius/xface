#include "crt.h"
#include "bsreq.h"
#include "io.h"

namespace {
struct context {
	io::stream&		file;
	bool			writemode;
	vector<void*>	references;
	void serial(void* v, unsigned lenght) {
		if(writemode)
			file.write(v, lenght);
		else
			file.read(v, lenght);
	}
	bool equal(const void* p1, const void* p2, const bsreq* type, const bsreq* stop_type) {
		for(auto p = type; *p; p++) {
			if(stop_type && stop_type == p)
				break;
			if(p->equal(p1, p2) != 0)
				return false;
		}
		return true;
	}
	void* findadd(array* pa, const void* pv, const bsreq* type, const bsreq* stop_type) {
		auto pe = pa->end();
		for(auto pb = pa->begin(); pb < pe; pb += pa->getsize()) {
			if(equal(pb, pv, type, stop_type))
				return pb;
		}
		return pa->add(pv);
	}
	void serial(void** pv, const bsreq* type, array* source) {
		unsigned i = 0;
		auto p = *pv;
		if(writemode) {
			if(p)
				i = references.indexof(p);
			file.write(&i, sizeof(i));
			if(i != 0xFFFFFFFF)
				return;
			references.add(p);
			if(type == bsmeta<const char*>::meta) {
				i = zlen((const char*)p);
				file.write(&i, sizeof(i));
				file.write(p, i);
			} else
				serial(p, type, type + 1);
		} else {
			char buffer[2048];
			file.read(&i, sizeof(i));
			if(!i)
				*pv = 0;
			else if(i != 0xFFFFFFFF)
				*pv = references[i];
			else {
				auto pb = buffer;
				if(type == bsmeta<const char*>::meta) {
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
					references.add(0);
					auto stop_type = type + 1;
					serial(pb, type, stop_type);
					references[pid] = findadd(source, pb, type, stop_type);
					*pv = references[pid];
				}
				if(pb != buffer)
					delete pb;
			}
		}
	}
	void serial_adat(adat<char, 4>* pv, const bsreq* type, unsigned size) {
		serial(&pv->count, sizeof(pv->count));
		for(unsigned j = 0; j < pv->count; j++)
			serial(pv->data + size*j, type);
	}
	void serial_rem(vector<char>* pv, const bsreq* type, unsigned size) {
		if(!writemode) {
			decltype(pv->getmaximum()) n = 0;
			serial(&n, sizeof(n));
			pv->reserve(n);
		} else {
			auto n = pv->getmaximum();
			serial(&n, sizeof(n));
		}
		serial(&pv->count, sizeof(pv->count));
		for(unsigned j = 0; j < pv->count; j++)
			serial(pv->ptr(j), type);
	}
	void serial(void* object, const bsreq* records, const bsreq* records_stop = 0) {
		for(auto p = records; *p; p++) {
			if(p == records_stop)
				break;
			switch(p->subtype) {
			case bsreq::kind::Number:
			case bsreq::kind::CFlags:
			case bsreq::kind::Enum:
				serial(p->ptr(object), p->lenght);
				break;
			case bsreq::kind::Text:
			case bsreq::kind::Reference:
				for(unsigned i = 0; i < p->count; i++)
					serial((void**)p->ptr(object, i), p->type, p->source);
				break;
			case bsreq::kind::Scalar:
				for(unsigned i = 0; i < p->count; i++)
					serial(p->ptr(object, i), p->type);
				break;
			case bsreq::kind::ADat:
				serial_adat(((adat<char, 4>*)p->ptr(object)), p->type, p->size);
				break;
			case bsreq::kind::ARem:
				serial_rem(((vector<char>*)p->ptr(object)), p->type, p->size);
				break;
			case bsreq::kind::List:
				break;
			}
		}
	}
	constexpr context(io::stream& file, bool writemode) : file(file), writemode(writemode) {}
};
}

bool bsreq::write(const char* url, void* object) const {
	io::file file(url, StreamWrite);
	if(!file)
		return false;
	context e(file, true);
	e.serial(object, this);
	return true;
}

bool bsreq::read(const char* url, const void* object) const {
	io::file file(url, StreamRead);
	if(!file)
		return false;
	context e(file, false);
	e.serial((void*)object, this);
	return true;
}