#include "xface/crt.h"
#include "xface/io.h"
#include "xface/strlib.h"
#include "main.h"

using namespace code;

struct metadata_context {
	struct headeri {
		char				signature[4];
		char				version[4];
	};
	headeri					header;
	io::stream&				file;
	arem<metadata*>&		types;
	arem<metadata>			types_read;
	bool					write_mode;

	metadata_context(io::stream& file, bool write_mode, arem<metadata*>& types) :
		header{"MTP", "0.1"}, file(file), write_mode(write_mode), types(types) {
	}

	template<class T> void serial(T& source) {
		serial(&source, sizeof(source));
	}

	void serial(void* object, unsigned size) {
		if(write_mode)
			file.write(object, size);
		else
			file.read(object, size);
	}

	void serial(const char*& ps) {
		unsigned len;
		if(write_mode) {
			if(ps)
				len = zlen(ps);
			file.write(&len, sizeof(len));
			if(len)
				file.write(ps, len);
		} else {
			file.read(&len, sizeof(len));
			ps = 0;
			char temp[256 * 32];
			char* ppt = temp;
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

	void serial(void* object, const requisit& e, const requisit* pid) {
		if(write_mode) {
			auto pv = *((void**)object);
			if(!pv)
				serial(&pv, sizeof(pv));
			else
				serial(pv, *pid);
		} else {
			const char* id = 0;
			serial(id);
			if(e.type->ismeta()) {
			}
		}
	}

	void serial(void* object, const requisit& e) {
		if(e.type->isreference()) {
			auto type = e.type->type;
			if(type->isnumber() || type->istext() || type->isreference())
				return;
			auto pid = type->getid();
			if(pid)
				serial(object, e, pid);
		} else if(e.type->isarray()) {
			auto pa = (arrayc*)object;
			auto sz = e.type->type->size;
			serial(pa->count);
			if(!write_mode) {
				if(pa->count > 0)
					pa->reserve(pa->count*sz);
			}
			for(unsigned i = 0; i < pa->count; i++)
				serial(pa->data + i * sz, e.type->type);
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
		for(auto& e : pm->requisits) {
			if(e.count <= 1)
				serial(e.ptr(object), e);
			else {
				for(unsigned i = 0; i < e.count; i++)
					serial(e.ptr(object, i), e);
			}
		}
	}

};

void metadata::write(const char* url, arem<metadata*>& types) {
	auto meta_type = config.types.find("Type");
	if(!meta_type)
		return;
	io::file file(url, StreamWrite);
	if(!file)
		return;
	metadata_context e(file, true, types);
	e.serial(e.header);
	e.serial(e.types.count);
	for(auto p : types) {
		if(p->isreference())
			continue;
		if(p->isarray())
			continue;
		e.serial(p, meta_type);
	}
}

void metadata::addto(arem<metadata*>& source) const {
	if(ispredefined())
		return;
	auto i = source.indexof(const_cast<metadata*>(this));
	if(i != -1)
		return;
	source.add(const_cast<metadata*>(this));
	for(auto& e : requisits) {
		if(e.type->type)
			e.type->type->addto(source);
		e.type->addto(source);
	}
}

void metadata::write(const char* url) const {
	arem<metadata*> types;
	addto(types);
	write(url, types);
}