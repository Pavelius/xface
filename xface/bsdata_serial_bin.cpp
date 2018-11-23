#include "archive.h"
#include "bsdata.h"

struct bsdata_archive : archive {
	
	arrayref					pointers[3];
	adat<bsreq, 2048>			requisits;
	amap<const bsreq*, const bsreq*>	types;

	constexpr bsdata_archive(io::stream& e, bool writemode) : requisits(), types(),
		pointers{requisits, number_type, text_type}, archive(e, writemode, pointers) {}

	const bsreq* get(const bsreq* fields) {
		if(fields->issimple())
			return fields;
		auto result = types.get(fields);
		if(result)
			return result;
		auto fields_copy = requisits.data + requisits.count;
		types.add(fields, fields_copy);
		auto fields_count = zlen(fields) + 1;
		memcpy(fields_copy, fields, fields_count * sizeof(bsreq));
		requisits.count += fields_count;
		for(int i = 0; i < fields_count - 1; i++)
			requisits.data[i].type = get(requisits.data[i].type);
		return fields_copy;
	}

	void set(void* data, unsigned size) override {
		archive::set(data, size);
	}

	void setfield(void* data, const bsreq* field) {
		for(unsigned i = 0; i < field->count; i++) {
			auto pv = (void*)field->ptr(data, i);
			if(field->type == number_type) {
				if(field->reference) {

				} else
					set(pv, field->lenght);
			} else if(field->type == text_type) {
				if(field->size==sizeof(const char*))
					setstring((const char**)pv);
			}
		}
	}

	void set(void* data, const bsreq* fields) {
		for(auto p = fields; *p; p++)
			setfield(data, p);
	}

	void serialize(array& source, const bsreq* fields) {
		get(fields);
		archive::set(requisits);
		archive::set(source.count);
		auto pb = source.begin();
		auto pe = source.end();
		for(pb = source.begin(); pb < pe; pb += source.size)
			set(pb, fields);
	}

};

template<> void archive::set<bsreq>(bsreq& e) {
	set(e.id);
	set(e.offset);
	set(e.size);
	set(e.lenght);
	set(e.count);
	set(e.type);
	set(e.reference);
	set(e.subtype);
}

void bsdata::write(const char* url, const array& source, const bsreq* fields) {
	io::file file(url, StreamWrite);
	if(!file)
		return;
	bsdata_archive a(file, true);
	if(!a.signature("MTD"))
		return;
	if(!a.version(1, 0))
		return;
	a.serialize(const_cast<array&>(source), fields);
}