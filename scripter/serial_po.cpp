#include "xface/crt.h"
#include "xface/io.h"
#include "main.h"

using namespace code;

namespace {
enum mode_s : unsigned char {
	WriteMode, ReadMode,
};
class serializator {
	struct element {
		void*			object;
		requisit*		collection;
	};
	arem<element>		references;
public:
	constexpr serializator() {}
	const element* find(const void* p) const {
		for(auto& e : references) {
			if(e.object == p)
				return &e;
		}
		return 0;
	}
	unsigned getfid(const void* p) {
		if(!p)
			return 0;
		auto pe = find(p);
		if(pe)
			return references.indexof(pe);
		return references.getcount();
	}
	void* getref(unsigned fid) const {
		return references[fid].object;
	}
	void makeref(const void* p) {
		if(!p)
			return;
		auto pe = find(p);
		if(!pe)
			return;
	}
};
struct context : serializator {
	struct headeri {
		char			signature[4];
		char			version[4];
	};
	headeri				header;
	io::stream&			file;
	mode_s				mode;
	context(io::stream& file, mode_s mode) : header{"MTD", "0.1"}, file(file), mode(mode) {}
	void serial(void* object, unsigned size) {
		switch(mode) {
		case WriteMode: file.write(object, size); break;
		case ReadMode: file.read(object, size); break;
		}
	}
	template<class T> void serial(T& e) {
		serial(&e, sizeof(e));
	}
	void serial(const char*& ps) {
		unsigned len = 0;
		char temp[256 * 32];
		char* ppt;
		switch(mode) {
		case WriteMode:
			if(ps)
				len = zlen(ps);
			file.write(&len, sizeof(len));
			if(len)
				file.write(ps, len);
			break;
		case ReadMode:
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
			break;
		}
	}
	void serial(void** p) {
		unsigned fid;
		switch(mode) {
		case WriteMode:
			fid = getfid(*p);
			serial(fid);
			break;
		case ReadMode:
			file.read(&fid, sizeof(fid));
			*p = getref(fid);
			break;
		}
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
};
}

void metadata::write(const char* url) const {
	io::file file(url, StreamWrite);
	if(!file)
		return;
	context e(file, WriteMode);
	e.serial(e.header);
}