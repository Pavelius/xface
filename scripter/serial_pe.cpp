#include "crt.h"
#include "io.h"
#include "main.h"

using namespace code;

namespace {
class context {
	io::stream&					file;
	typedef adat<requisit*, 64> keya;
	vector<void*>				references;
	bool						writemode;
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
			if(memcmp((char*)p1 + p->offset, (char*)p2 + p->offset, p->getlenght()) != 0)
				return false;
		}
		return true;
	}
	static void copy(const void* p1, const void* p2, const keya& keys) {
		for(auto p : keys) {
			auto s = p->getlenght();
			auto u = p->offset;
			memcpy((char*)p1 + u, (char*)p2 + u, s);
		}
	}
	void* findbykey(array& source, void* p, const keya& k1) {
		for(int i = source.getcount() - 1; i >= 0; i--) {
			auto p1 = source.ptr(i);
			if(isequal(p1, p, k1))
				return p1;
		}
		return 0;
	}
	void select(keya& k, const metadata* type, bool key_only) {
		for(auto p : references) {
			if(bsdata<requisit>::source.indexof(p) == -1)
				continue;
			auto& e = *((requisit*)p);
			if(e.parent != type)
				continue;
			if(e.is(Static))
				continue;
			if(key_only && !e.is(Dimension))
				continue;
			k.add(&e);
		}
	}
	void* readobject(bool* read_result = 0, bool only_key = false) {
		char temp[256 * 4 * 4];
		char* p = temp;
		void* v = 0;
		unsigned i = 0;
		file.read(&i, sizeof(i));
		if(!i)
			return 0;
		auto type = (metadata*)references[i];
		if(!type)
			return 0;
		if(type->istext()) {
			file.read(&i, sizeof(i));
			if(i >= sizeof(temp))
				p = new char[i + 1];
			file.read(p, i);
			p[i] = 0;
			if(read_result)
				*read_result = true;
			v = (void*)szdup(p);
			references.add(v);
		} else {
			auto pa = type->getelements();
			if(!pa)
				return 0;
			keya keys; select(keys, type, only_key);
			references.add(0);
			auto pv1 = &references[references.getcount() - 1];
			serial(p, type, only_key);
			v = findbykey(*pa, p, keys);
			if(!v) {
				v = pa->add();
				memset(v, 0, pa->getsize());
			}
			*pv1 = v;
			if(!v)
				return 0;
			copy(v, p, keys);
		}
		if(p != temp)
			delete p;
		if(read_result)
			*read_result = true;
		return v;
	}
	void serial(void** object, const metadata* type) {
		unsigned i = 0;
		if(writemode) {
			auto p = *object;
			if(p)
				i = references.indexof(p);
			file.write(&i, sizeof(i));
			if(i != 0xFFFFFFFF)
				return;
			writeobject(p, type, true);
		} else {
			file.read(&i, sizeof(i));
			if(!i)
				*object = 0;
			else if(i != 0xFFFFFFFF)
				*object = references[i];
			else
				*object = readobject(0, true);
		}
	}
	void serial(void* object, const requisit& e) {
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
		for(auto p : references) {
			if(bsdata<requisit>::source.indexof(p) == -1)
				continue;
			auto& e = *((requisit*)p);
			if(e.parent != pm)
				continue;
			if(e.is(Static))
				continue;
			if(key_only && !e.flags.is(Dimension))
				continue;
			serial(e.ptr(object), e);
		}
	}
	void write_requisits(const metadata* m) {
		for(auto& e : bsdata<requisit>()) {
			if(e.parent != m)
				continue;
			writeobject(&e, metadata::type_requisit, false);
		}
	}
	void writeobject(const void* m, const metadata* type, bool key_only) {
		if(references.indexof((void*)m) != -1)
			return;
		references.add((void*)m);
		unsigned i = (unsigned)references.indexof((void*)type);
		file.write(&i, sizeof(i));
		if(type->istext()) {
			i = zlen((const char*)m);
			file.write(&i, sizeof(i));
			file.write(m, i);
		} else
			serial((void*)m, type, key_only);
	}
	void write_type(const metadata* m) {
		if(references.indexof((void*)m) != -1)
			return;
		if(m->isreference())
			write_type(m->type);
		writeobject(m, metadata::type_metadata, false);
		write_requisits(m);
	}
public:
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
			write_requisits((metadata*)p);
		}
	}
	void read() {
		bool allok = true;
		while(allok) {
			allok = false;
			readobject(&allok, false);
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

void metadata::read(const char* url) {
	io::file file(url, StreamRead);
	if(!file)
		return;
	context e(file, false);
	e.read();
}