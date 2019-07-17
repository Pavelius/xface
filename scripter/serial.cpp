#include "xface/io.h"
#include "xface/strlib.h"
#include "main.h"

using namespace code;

struct metadata_context {
	
	struct slicei {
		unsigned			offset;
		unsigned			count;
	};
	struct headeri {
		char				signature[4];
		char				version[4];
		slicei				metadata;
		slicei				strings;
	};

	headeri					header;
	io::stream&				file;
	strlib					strings;
	metadata::typec&		types;
	bool					write_mode;

	metadata_context(io::stream& file, bool write_mode, metadata::typec& types) :
		header{"MTP", "0.1"}, file(file), write_mode(write_mode), types(types) {
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
			if(i==0xFFFFFFFF)
				*((metadata**)object) = 0;
			else
				*((metadata**)object) = (metadata*)types.data + i;
		}
	}

	void serial_txt(void* object) {
		if(write_mode) {
			unsigned i = strings.add(*((const char**)object));
			file.write(&i, sizeof(i));
		} else {
			unsigned i = 0xFFFFFFFF;
			file.read(&i, sizeof(i));
			*((const char**)object) = strings.get(i);
		}
	}

	void serial(void* object, const requisit& e) {
		if(e.type->istext())
			serial_txt(object);
		else if(e.type->is("Type"))
			serial_met(object);
		else
			serial(object, e.type->size);
	}

	void serial(void* object, const metadata* pm) {
		if(!pm)
			return;
		for(auto& e : pm->requisits)
			serial(object, e);
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
	e.header.metadata.offset = sizeof(e.header);
	e.header.metadata.count = types.count;
	e.serial(&e.header, sizeof(e.header));
	for(auto& m : types)
		e.serial(&m, meta_type);
}

void metadata::addto(metadata::typec& source) const {
	for(auto& e : requisits) {
		if(e.type->ispredefined())
			continue;
		e.type->addto(source);
	}
	auto i = source.indexof(const_cast<metadata*>(this));
	if(i == -1)
		source.add(const_cast<metadata*>(this));
}

void metadata::write(const char* url) const {
	arem<metadata*> types;
	addto(types);
	write(url, types);
}