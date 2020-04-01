#include "xface/crt.h"
#include "xface/io.h"
#include "main.h"

using namespace code;

namespace {
class context {
	typedef adat<requisit*, 16> keya;
	io::stream&			file;
	bool				writemode;
	arem<void*>			references;
	void serial(void* object, unsigned size) {
		if(writemode)
			file.write(object, size);
		else
			file.read(object, size);
	}
	void serial(unsigned& object) {
		serial(&object, sizeof(object));
	}
	static bool isequal(const void* p1, const void* p2, const keya& keys) {
		for(auto p : keys) {
			if(memcmp((char*)p1 + p->offset, (char*)p2 + p->offset, p->count*p->parent->size) != 0)
				return false;
		}
		return true;
	}
	void* findbykey(array& source, void* p, const keya& k1) {
		for(auto i = source.getcount() - 1; i >= 0; i--) {
			auto p1 = source.ptr(i);
			if(isequal(p1, p, k1))
				return p1;
		}
		return 0;
	}
	void* findbykey(void* p, const metadata* type) {
		keya k1;
		for(auto p : references) {
			if(bsdata<requisit>::source.indexof(p) == -1)
				continue;
			if(((requisit*)p)->parent != type)
				continue;
			if(!((requisit*)p)->flags.is(Dimension))
				continue;
			k1.add((requisit*)p);
		}
		return 0;
	}
	void serial(void** object, const metadata* type) {
		if(writemode) {
			unsigned i = 0;
			auto p = *object;
			if(p)
				i = references.indexof(p);
			file.write(&i, sizeof(i));
			if(i != 0xFFFFFFFF)
				return;
			references.add(p);
			i = references.indexof((void*)type);
			file.write(&i, sizeof(i));
			if(type->istext()) {
				i = zlen((const char*)p);
				file.write(&i, sizeof(i));
				file.write(p, i);
			} else
				serial(p, type, true);
		} else {
			unsigned i = 0;
			file.read(&i, sizeof(i));
			if(!i)
				*object = 0;
			else if(i!=0xFFFFFFFF)
				*object = references[i];
			else {
				char temp[256 * 4];
				char* p = temp;
				file.read(&i, sizeof(i));
				type = (metadata*)references[i];
				if(type->istext()) {
					file.read(&i, sizeof(i));
					if(i >= sizeof(temp))
						p = new char[i + 1];
					file.read(p, sizeof(i));
					p[i] = 0;
					*object = (void*)szdup(p);
				} else {
					serial(p, type, true);
					*object = findbykey(p, type);
				}
				references.add(*object);
				if(p != temp)
					delete p;
			}
		}
	}
	void serial(void* object, const requisit& e) {
		if(writemode)
			write_type(e.type);
		if(e.type->isreference()) {
			auto type = e.type->type;
			if(type->isnumber() || type->istext() || type->isreference())
				return; // Skip serialization of this hard case
			serial((void**)object, type);
		} else if(e.type->isarray()) {

		} else if(e.type->istext())
			serial((void**)object, metadata::type_text);
		else if(e.type->isnumber())
			serial(object, e.type->size);
		else
			serial(object, e.type, false);
	}
	void serial(void* object, const metadata* pm, bool key_only) {
		if(!object)
			return;
		for(auto& e : bsdata<requisit>()) {
			if(e.parent != pm)
				continue;
			if(key_only && !e.flags.is(Dimension))
				continue;
			serial(e.ptr(object), e);
		}
	}
	void write_requisits(const metadata* m) {
		unsigned fid = metadata::type_requisit->getid();
		for(auto& e : bsdata<requisit>()) {
			if(e.parent != m)
				continue;
			serial(fid);
			serial(&e, metadata::type_requisit, false);
		}
	}
	void write_type(const metadata* m) {
		if(references.indexof((void*)m) != -1)
			return;
		if(m->isreference())
			write_type(m->type);
		references.add((void*)m);
		unsigned fid = metadata::type_metadata->getid();
		serial(fid);
		serial((void*)m, metadata::type_metadata, false);
		write_requisits(m);
	}
public:
	context(io::stream& file, bool writemode) : file(file), writemode(writemode) {
		for(auto& e : bsdata<metadata>()) {
			if(!e.ispredefined())
				break;
			references.add(&e);
		}
	}
	void writemeta(const metadata* m) {
		auto start = references.getcount();
		write_type(m);
		auto end = references.getcount();
		for(auto i = start, start = end; i < end; i++) {
			auto p = references[i];
			if(p == m)
				continue;
			if(bsdata<metadata>::source.indexof(p) == -1)
				continue;
			write_requisits(m);
		}
	}
};
}

void metadata::write(const char* url) const {
	io::file file(url, StreamWrite);
	if(!file)
		return;
	context e(file, true);
	e.writemeta(this);
}