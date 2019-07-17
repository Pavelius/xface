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

	void serial_ref(void* object, const requisit& e) {

	}

	void serial(void* object, const requisit& e) {
		if(e.isreference()) {
			if(e.type->ispredefined())
				return; // Не поддерживается
			auto pid = e.type->getid();
			if(!pid)
				return;
			serial_ref(object, e);
		} else if(e.type->istext())
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

	void serial_strings() {
		serial(strings.indecies);
		serial(strings.data);
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
		e.serial(&m, meta_type);
	e.serial_strings();

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