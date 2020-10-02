#include "crt.h"
#include "io.h"
#include "main.h"

using namespace code;

namespace {
class context {
	io::stream&					file;
	vector<void*>				references;
	bool						writemode;
	bool isref(const void* v) const {
		return references.indexof(const_cast<void*>(v)) != -1;
	}
	static bool isequal(const void* p1, const void* p2, const aref<requisit*>& keys) {
		for(auto p : keys) {
			if(memcmp((char*)p1 + p->offset, (char*)p2 + p->offset, p->getlenght()) != 0)
				return false;
		}
		return true;
	}
	void copy(const void* p1, const void* p2, const metadata* type) {
		for(auto pv : references) {
			if(!bsdata<requisit>::source.is(pv))
				continue;
			auto p = (requisit*)pv;
			if(p->is(Static))
				continue;
			if(p->parent != type)
				continue;
			auto s = p->getlenght();
			auto u = p->offset;
			memcpy((char*)p1 + u, (char*)p2 + u, s);
		}
	}
	void* findbykey(array& source, void* p, const aref<requisit*>& k1) {
		for(int i = source.getcount() - 1; i >= 0; i--) {
			auto p1 = source.ptr(i);
			if(isequal(p1, p, k1))
				return p1;
		}
		return 0;
	}
	unsigned select(requisit** result, requisit** pe, const metadata* type) {
		auto pb = result;
		for(auto pv : references) {
			if(!bsdata<requisit>::source.is(pv))
				continue;
			auto p = (requisit*)pv;
			if(p->parent != type || p->is(Static))
				continue;
			if(!p->is(Dimension))
				continue;
			*pb++ = p;
			if(pb >= pe)
				break;
		}
		return pb - result;
	}
	unsigned getref(const void* m) {
		return references.indexof(const_cast<void*>(m));
	}
	void serial_text(const char** m) {
		if(writemode) {
			auto p = *m;
			if(!p)
				p = "";
			unsigned i = zlen(p);
			file.write(&i, sizeof(i));
			if(i)
				file.write(*m, i);
		} else {
			char temp[4096];
			auto p = temp;
			unsigned i = 0;
			file.read(&i, sizeof(i));
			if(i != 0) {
				if(i >= sizeof(temp) - 1)
					p = new char[i + 1];
				file.read(p, i);
				p[i] = 0;
				*m = szdup(p);
				references.add(m);
				if(p != temp)
					delete p;
			} else
				*m = 0;
		}
	}
public:
	void write_trail() {
		unsigned i = 0;
		if(writemode)
			file.write(&i, sizeof(i));
	}
	void serial_ref(void** m, const metadata* type) {
		if(writemode) {
			if(*m == 0)
				file.write(m, sizeof(void*));
			else {
				auto i = getref(*m);
				file.write(&i, sizeof(i));
				if(i != 0xFFFFFFFF)
					return;
				references.add(*m);
				serial_ref((void**)&type, metadata::type_metadata);
				serial(*m, type);
			}
		} else {
			unsigned i = 0;
			file.read(&i, sizeof(i));
			if(i == 0)
				*m = 0;
			else if(i != 0xFFFFFFFF)
				*m = references[i];
			else {
				metadata* new_type = 0;
				serial_ref((void**)&new_type, metadata::type_metadata);
				if(!new_type)
					return;
				char temp[1024];
				auto p = temp;
				if(new_type->size > sizeof(temp))
					p = new char[new_type->size];
				serial(p, new_type);
				auto pa = new_type->getelements();
				requisit* keys[8];
				auto keys_count = select(keys, keys + sizeof(keys)/sizeof(keys[0]), new_type);
				auto pf = findbykey(*pa, p, {keys, keys_count});
				if(!pf)
					pf = pa->addz();
				copy(pf, p, new_type);
				*m = pf;
				if(p != temp)
					delete p;
			}
		}
	}
	void serial(void* m, const metadata* type) {
		if(type == metadata::type_text)
			serial_text((const char**)m);
		else if(type->isnumber()) {
			if(writemode)
				file.write(m, type->size);
			else
				file.read(m, type->size);
		} else if(type->isreference()) {
			if(type->type->isreference())
				return;
			//if(type->isnumber() || type->istext() || type->isreference())
			//	return; // Skip serialization of this hard case
			serial_ref((void**)m, type->type);
		} else if(type->isarray()) {
			auto pa = (array*)m;
			if(writemode) {
				file.write(&pa->count, sizeof(pa->count));
				auto pe = pa->end();
				for(auto p = pa->begin(); p < pe; p += pa->getsize())
					serial(p, type->type);
			} else {
				unsigned count = 0;
				file.read(&count, sizeof(count));
				pa->clear();
				pa->reserve(count);
				for(unsigned i = 0; i < count; i++) {
					auto p = pa->addz();
					serial(p, type->type);
				}
			}
		} else {
			for(auto pv : references) {
				if(!bsdata<requisit>::source.is(pv))
					continue;
				auto p = (requisit*)pv;
				if(p->parent != type)
					continue;
				if(p->is(Static))
					continue;
				//if(key_only && !e.flags.is(Dimension))
				//	continue;
				serial(p->ptr(m), p->type);
			}
		}
	}
	context(io::stream& file, bool writemode) : file(file), writemode(writemode) {
		for(auto& e : bsdata<metadata>()) {
			if(!e.ispredefined())
				break;
			references.add(&e);
		}
		for(auto& e : bsdata<requisit>()) {
			if(!e.ispredefined())
				break;
			references.add(&e);
		}
		references.add((void*)szdup("*")); // Recently used pointer name
		references.add((void*)szdup("&")); // Recently used array name
		// Reserve some metadata for future use
		while(references.getcount() < 32)
			references.add(0);
	}
};
}

void metadata::write(const char* url) const {
	io::file file(url, StreamWrite);
	if(!file)
		return;
	context en(file, true);
	auto ptr_type = this;
	en.serial_ref((void**)&ptr_type, metadata::type_metadata);
	for(auto& e : bsdata<requisit>()) {
		if(!e)
			continue;
		if(e.parent != this)
			continue;
		auto ptr_requisit = &e;
		en.serial_ref((void**)&ptr_requisit, metadata::type_requisit);
	}
	en.write_trail();
}

void metadata::read(const char* url) {
	io::file file(url, StreamRead);
	if(!file)
		return;
	context en(file, false);
	while(true) {
		void* object = 0;
		en.serial_ref(&object, 0);
		if(!object)
			break;
	}
}