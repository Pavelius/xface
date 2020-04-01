#include "xface/crt.h"
#include "xface/io.h"
#include "main.h"

using namespace code;

namespace {
class context {
	io::stream&			file;
	bool				writemode;
	arem<void*>			references;
	unsigned			fid_void, fid_text, fid_metadata, fid_requisit;
	void serial(void* object, unsigned size) {
		if(writemode)
			file.write(object, size);
		else
			file.read(object, size);
	}
	void serial(unsigned& object) {
		serial(&object, sizeof(object));
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
			if(fid_text == i) {
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
				if(i == fid_text) {
					file.read(&i, sizeof(i));
					if(i >= sizeof(temp))
						p = new char[i + 1];
					file.read(p, sizeof(i));
					p[i] = 0;
					*object = (void*)szdup(p);
				} else {
					serial(p, type, true);
					// TODO: find element by keys
				}
				if(p != temp)
					delete p;
			}
		}
	}
	void write_requisits(const metadata* m) {
		auto type_requisit = (metadata*)references[fid_requisit];
		for(auto& e : bsdata<requisit>()) {
			if(e.parent != m)
				continue;
			serial(fid_requisit);
			serial(&e, type_requisit, false);
		}
	}
	void write_type(const metadata* m) {
		if(references.indexof((void*)m) != -1)
			return;
		if(m->isreference())
			write_type(m->type);
		references.add((void*)m);
		serial(fid_metadata);
		serial((void*)m, (metadata*)references[fid_metadata], false);
		write_requisits(m);
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
			serial((void**)object, (metadata*)references[fid_text]);
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
	unsigned addmeta(const char* id) {
		auto p = addtype(id);
		auto i = references.getcount();
		references.add(p);
		return i;
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
public:
	context(io::stream& file, bool writemode) : file(file), writemode(writemode) {
		fid_void = addmeta("Void");
		addmeta("Char");
		addmeta("Byte");
		addmeta("Short");
		addmeta("Short Unsigned");
		addmeta("Integer");
		addmeta("Unsigned");
		fid_text = addmeta("Text");
		fid_metadata = addmeta("Type");
		fid_requisit = addmeta("Requisit");
		addmeta("*Type");
	}
	void test() {
		auto p = addtype("Character");
		writemeta(p);
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