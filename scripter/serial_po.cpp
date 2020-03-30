#include "xface/crt.h"
#include "xface/io.h"
#include "main.h"

using namespace code;

namespace {
class context {
	struct element {
		const void*		object;
	};
	struct headeri {
		char			signature[4];
		char			version[4];
	};
	headeri				header;
	io::stream&			file;
	bool				write_mode;
	arem<element>		references;
	const element* find(const void* p) const {
		for(auto& e : references) {
			if(e.object == p)
				return &e;
		}
		return 0;
	}
	bool isexist(const void* p) const {
		return find(p) != 0;
	}
	unsigned getfid(const void* p) {
		if(!p)
			return 0;
		element* pe = (element*)find(p);
		if(pe)
			return pe - references.data;
		pe = references.add();
		pe->object = p;
		return pe - references.data;
	}
	const void* getref(unsigned fid) const {
		return references.data[fid].object;
	}
	void addref(const void* p) {
		if(!p)
			return;
		if(isexist(p))
			return;
		auto pe = references.add();
		pe->object = p;
	}
	void addref(const metadata* p) {
		if(p && p->type)
			addref(p->type);
		addref((const void*)p);
	}
	void serial(void* object, unsigned size) {
		if(write_mode)
			file.write(object, size);
		else
			file.read(object, size);
	}
	template<class T> void serial(T& e) {
		serial(&e, sizeof(e));
	}
	void serial(const char*& ps) {
		unsigned len = 0;
		char temp[256 * 32];
		char* ppt;
		if(write_mode) {
			if(ps)
				len = zlen(ps);
			file.write(&len, sizeof(len));
			if(len)
				file.write(ps, len);
		} else {
			file.read(&len, sizeof(len));
			ps = 0;
			ppt = temp;
			if(len > 0) {
				if(len > sizeof(temp) - 1)
					ppt = new char[len + 1];
				file.read(ppt, len);
			}
			ppt[len] = 0;
			ps = szdup(ppt);
			if(ppt != temp)
				delete ppt;
		}
	}
	void serial(void** p) {
		unsigned fid;
		if(write_mode) {
			fid = getfid(*p);
			serial(fid);
		} else {
			file.read(&fid, sizeof(fid));
			if(!fid)
				*p = 0;
			else
				*p = (void*)references.data[fid].object;
		}
	}
	void serial(element& e) {
	}
	void serial(void* object, const requisit& e) {
		if(e.type->isreference()) {
			auto type = e.type->type;
			if(type->isnumber() || type->istext() || type->isreference())
				return;
			serial((void**)object);
		} else if(e.type->isarray()) {

		} else if(e.type->istext())
			serial(*((const char**)object));
		else if(e.type->isnumber())
			serial(object, e.type->size);
		else
			serial(object, e.type);
	}
	void serial(void* object, const metadata* pm) {
		if(!pm)
			return;
		for(auto& e : bsdata<requisit>()) {
			if(e.parent != pm)
				continue;
			if(e.count <= 1)
				serial(e.ptr(object), e);
			else {
				for(unsigned i = 0; i < e.count; i++)
					serial(e.ptr(object, i), e);
			}
		}
	}
public:
	context(io::stream& file, bool write_mode) : header{"MTD", "0.1"}, file(file), write_mode(write_mode) {}
};
}

void metadata::write(const char* url) const {
	io::file file(url, StreamWrite);
	if(!file)
		return;
	context e(file, true);
	auto p = addtype("*Character");
	//e.serial(e.header);
}