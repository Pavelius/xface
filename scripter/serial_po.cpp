#include "xface/crt.h"
#include "xface/io.h"
#include "main.h"

using namespace code;

namespace {
class context {
	io::stream&			file;
	bool				write_mode, refscan;
	arem<void*>			references;
	unsigned			fid_void, fid_text, fid_metadata, fid_requisit;
	bool isexist(const void* p) const {
		return references.indexof((void*)p) != -1;
	}
	unsigned getfid(const char* p) {
		if(!p)
			return 0;
		p = szdup(p);
		auto i = references.indexof((void*)p);
		if(i != -1)
			return i;
		i = references.getcount();
		references.add((void*)p);
		file.write(fid_text);
		unsigned len = zlen(p);
		file.write(len);
		file.write(p, len);
		return i;
	}
	unsigned getfid(const void* p) {
		if(!p)
			return 0;
		auto i = references.indexof((void*)p);
		if(i != -1)
			return i;
		i = references.getcount();
		references.add((void*)p);
		return i;
	}
	unsigned addmeta(const char* pn) {
		if(!pn)
			return 0;
		auto p = addtype(pn);
		auto i = references.indexof((void*)p);
		if(i != -1)
			return i;
		i = references.getcount();
		references.add((void*)p);
		return i;
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
	void serial(void* object, const requisit& e) {
		if(e.type->isreference()) {
			auto type = e.type->type;
			if(type->isnumber() || type->istext() || type->isreference())
				return; // Skip serialization of this hard case
			if(refscan) {
				auto p = *((void**)object);
				//if(p)
				//	serial(p, type);
			}
		} else if(e.type->isarray()) {

		} else if(e.type->istext()) {
			if(refscan)
				getfid(*((const char**)object));
			else
				serial(*((const char**)object));
		} else if(e.type->isnumber()) {
			if(!refscan)
				serial(object, e.type->size);
		} else
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
	context(io::stream& file, bool write_mode) : file(file), write_mode(write_mode) {
		addmeta("Void");
		addmeta("Char");
		addmeta("Byte");
		addmeta("Short");
		addmeta("Short Unsigned");
		addmeta("Integer");
		addmeta("Unsigned");
		fid_text = addmeta("Text");
		fid_metadata = addmeta("Type");
		fid_requisit = addmeta("Requisit");
	}
	void serial(const metadata* p) {
		serial((void*)p, (metadata*)references[fid_metadata]);
		for(auto& e : bsdata<requisit>()) {
			if(!e || e.parent != p)
				continue;
			serial(&e, (metadata*)references[fid_requisit]);
		}
	}
	void test() {
		refscan = true;
		serial(addtype("Character"));
	}
};
}

void metadata::write(const char* url) const {
	io::file file(url, StreamWrite);
	if(!file)
		return;
	context e(file, true);
	e.test();
}