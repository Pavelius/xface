#include "xface/crt.h"
#include "xface/io.h"
#include "xface/strlib.h"
#include "main.h"

using namespace code;

struct metadata_context {

	typedef arem<char>		arrayc;
	struct slicei {
		unsigned			offset;
		unsigned			count;
	};
	struct headeri {
		char				signature[4];
		char				version[4];
	};
	headeri					header;
	io::stream&				file;
	arem<metadata*>			types;
	arem<metadata>			types_read;
	bool					write_mode;

	metadata_context(io::stream& file, bool write_mode, arem<metadata*>& types) :
		header{"MTP", "0.1"}, file(file), write_mode(write_mode), types(types) {
	}

	metadata* findtype(const char* id) const {
		for(auto p : types) {
			if(strcmp(p->id, id) == 0)
				return const_cast<metadata*>(p);
		}
		return 0;
	}

	void serial(void* object, unsigned size) {
		if(write_mode)
			file.write(object, size);
		else
			file.read(object, size);
	}

	void serial_met(void* object) {
		if(write_mode) {
			unsigned i = types.indexof(*((metadata**)object));
			file.write(&i, sizeof(i));
		} else {
			unsigned i = 0xFFFFFFFF;
			file.read(&i, sizeof(i));
			if(i == 0xFFFFFFFF)
				*((metadata**)object) = 0;
			else
				*((metadata**)object) = (metadata*)types.data + i;
		}
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
				auto pt = findtype(id);
				if(!pt)
					pt = addtype(id);
			}
		}
	}

	void serial(void* object, const requisit& e) {
		if(e.type->isreference()) {
			auto type = e.type->dereference();
			if(type->isnumber() || type->istext())
				return;
			auto pid = type->getid();
			if(pid)
				serial(object, e, pid);
		} else if(e.type->isarray()) {
			auto pa = (arrayc*)object;
			serial(pa->count);
			if(!write_mode) {
				if(pa->count>0)
					pa->reserve(pa->count*e.type->size);
			}
			for(unsigned i = 0; i < pa->count; i++)
				serial(pa->data + i * e.type->size, e.type);
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

	void serial(void** data, unsigned size, unsigned& count, unsigned& count_maximum) {
		serial(&count, sizeof(count));
		if(write_mode)
			serial(*data, count*size);
		else {

		}
	}

	template<class T> void serial(arem<T>& source) {
		serial((void**)&source.data, sizeof(source.data[0]), source.count, source.count_maximum);
	}

	template<class T> void serial(T& source) {
		serial(&source, sizeof(source));
	}

};

void metadata::write(const char* url, arem<metadata*>& types) {
	auto meta_type = findtype("Type");
	if(!meta_type)
		return;
	io::file file(url, StreamWrite);
	if(!file)
		return;
	metadata_context e(file, true, types);
	e.serial(e.header);
	e.serial(e.types.count);
	for(auto& m : types)
		e.serial(m, meta_type);
}

void metadata::addto(arem<metadata*>& source) const {
	auto i = source.indexof(const_cast<metadata*>(this));
	if(i != -1)
		return;
	source.add(const_cast<metadata*>(this));
	for(auto& e : requisits) {
		if(e.type->ispredefined())
			continue;
		e.type->addto(source);
	}
}

void metadata::write(const char* url) const {
	arem<metadata*> types;
	addto(types);
	write(url, types);
}