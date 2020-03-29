#include "xface/crt.h"
#include "xface/io.h"
#include "xface/strlib.h"
#include "main.h"

using namespace code;

void* seriala::getref(unsigned fid) const {
	return types[fid >> 24].references[fid && 0xFFFFFF];
}

seriali* seriala::find(const void* p) {
	for(auto& e : types) {
		if(!e.source)
			continue;
		if(e.source->indexof(p) == -1)
			continue;
		return &e;
	}
	return 0;
}

unsigned seriala::getfid(const void* p) {
	if(!p)
		return 0;
	auto pe = find(p);
	if(!pe)
		return 0;
	unsigned index = pe->references.indexof((void*)p);
	if(index == 0xFFFFFFFF) {
		index = pe->references.getcount();
		pe->references.add((void*)p);
	}
	return ((unsigned)types.indexof(pe) << 24) | index;
}

void seriala::makeref(const void* p) {
	if(!p)
		return;
	auto pe = find(p);
	if(!pe)
		return;
	if(pe->references.indexof((void*)p) != -1)
		return;
	pe->references.add((void*)p);
	if(pe->meta) {
		auto count = pe->keys;
		if(!count)
			count = 1;
		for(auto& e : bsdata<requisit>()) {
			if(e.parent != pe->meta)
				continue;
			makeref(e.type);
			if(--count == 0)
				break;
		}
	}
}

namespace {
enum mode_s : unsigned char {
	MakeReferences, WriteMode, ReadMode,
};
struct context : seriala {
	struct headeri {
		char			signature[4];
		char			version[4];
	};
	headeri				header;
	io::stream&			file;
	mode_s				mode;
	context(io::stream& file, mode_s mode, seriala& types) :
		seriala(types), header{"MTD", "0.1"}, file(file), mode(mode) {}
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
		case MakeReferences:
			makeref(*p);
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
	void serial(seriala& col) {
		serial(col.types.count);
		for(auto& e : col.types) {
			serial(e.references.count);
			for(auto p : e.references)
				serial(p, e.meta);
		}
	}
};
}

static void prepare_types(io::file& file, seriala& types, const metadata* element) {
	context e(file, MakeReferences, types);
	e.serial((void**)&element);
}

void metadata::write(const char* url, seriala& types, const metadata* element) {
	io::file file(url, StreamWrite);
	if(!file)
		return;
	prepare_types(file, types, element);
	context e(file, WriteMode, types);
	e.serial(e.header);
	e.serial(types);
}

void metadata::write(const char* url) const {
	seriali elements[4] = {{},
	{addtype("Type"), bsdata<metadata>::source_ptr},
	{addtype("Requisit"), bsdata<requisit>::source_ptr}
	};
	aref<seriali> source(elements);
	seriala	types(source);
	write(url, types, this);
}