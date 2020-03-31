#include "xface/crt.h"
#include "xface/io.h"
#include "main.h"

using namespace code;

namespace {
class context {
	io::stream&			file;
	bool				readmode, writemode, strmode;
	arem<void*>			references;
	arem<void*>			forward;
	unsigned			fid_void, fid_text, fid_metadata, fid_requisit;
	unsigned			start_str, start_metadata, start_requisit, start_objects;
	void clearfwd() {
		forward.clear();
		for(auto p : references)
			forward.add(p);
	}
	bool addfwd(const void* p) {
		if(!p)
			return true;
		auto i = forward.indexof((void*)p);
		if(i != -1)
			return true;
		forward.add((void*)p);
		return false;
	}
	void addref(const void* p) {
		if(!p)
			return;
		auto i = references.indexof((void*)p);
		if(i != -1)
			return;
		references.add((void*)p);
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
		if(writemode)
			file.write(object, size);
		else if(readmode)
			file.read(object, size);
	}
	void serial(const char*& ps) {
		char temp[256 * 32];
		unsigned len;
		char* ppt;
		if(strmode) {
			addref((void*)ps);
		} else if(writemode) {
			len = 0;
			if(ps)
				len = zlen(ps);
			file.write(&len, sizeof(len));
			if(len)
				file.write(ps, len);
		} else if(readmode) {
			len = 0;
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
	void serial(const metadata* m) {
		if(addfwd(m))
			return;
		if(m->isreference())
			serial(m->type);
		for(auto& e : bsdata<requisit>()) {
			if(e.parent != m)
				continue;
			serial(&e, (metadata*)references[fid_requisit]);
		}
		serial((void*)m, (metadata*)references[fid_metadata]);
	}
	void serial(void* object, const requisit& e) {
		serial(e.type);
		if(e.type->isreference()) {
			auto type = e.type->type;
			if(type->isnumber() || type->istext() || type->isreference())
				return; // Skip serialization of this hard case
			serial(*((void**)object), type);
		} else if(e.type->isarray()) {

		} else if(e.type->istext())
			serial(*((const char**)object));
		else if(e.type->isnumber())
			serial(object, e.type->size);
		else
			serial(object, e.type);
	}
	void serial(void* object, const metadata* pm) {
		if(!object)
			return;
		for(auto& e : bsdata<requisit>()) {
			if(e.parent != pm)
				continue;
			serial(e.ptr(object), e);
		}
	}
	void writestr() {
		for(auto i = start_str; i < start_metadata; i++) {
			auto p = (const char*)references[i];
			file.write(p, zlen(p) + 1);
		}
	}
public:
	context(io::stream& file) : file(file),
		writemode(false), readmode(false), strmode(false),
		start_str(0), start_metadata(0), start_requisit(0), start_objects(0),
		fid_metadata(0), fid_void(0), fid_requisit(0), fid_text(0) {
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
	void test() {
		strmode = true;
		clearfwd();
		start_str = references.getcount();
		serial(addtype("Character"));
		start_metadata = references.getcount();
		writestr();
	}
};
}

void metadata::write(const char* url) const {
	io::file file(url, StreamWrite);
	if(!file)
		return;
	context e(file);
	e.test();
}