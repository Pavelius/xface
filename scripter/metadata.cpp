#include "xface/crt.h"
#include "main.h"

const unsigned pointer_size = 4;

metadata text_meta[] = {"Text"};
metadata int_meta[] = {"Integer"};
metadata uint_meta[] = {"Unsigned"};
metadata sint_meta[] = {"Short"};
metadata usint_meta[] = {"Short Unsigned"};
metadata char_meta[] = {"Char"};
metadata uchar_meta[] = {"Byte"};

adat<requisit, 256 * 16>		requisit_data;
adat<metadata, 256 * 4>			metadata_data;
static adat<metadata, 256 * 8>	pointers;

metadata* findpointer(metadata* m) {
	for(auto& e : pointers) {
		if(e.type==m)
			return &e;
	}
	return 0;
}

metadata* findtype(const char* id) {
	for(auto& e : metadata_data) {
		if(strcmp(e.id, id) == 0)
			return &e;
	}
	return 0;
}

metadata* addtype(const char* id) {
	auto p = findtype(id);
	if(!p)
		p = metadata_data.add();
	p->id = id;
	p->size = 0;
	p->type = 0;
	return p;
}

metadata* metadata::reference() {
	auto p = findpointer(this);
	if(p)
		return p;
	p = pointers.add();
	p->type = this;
	p->id = "*";
	p->size = pointer_size;
	return p;
}

metadata* metadata::dereference() {
	return type;
}

requisit* metadata::add(const char* id, metadata* type) {
	id = szdup(id);
	auto p = find(id);
	if(!p)
		p = requisit_data.add();
	p->id = id;
	p->parent = this;
	p->type = type;
	p->count = 1;
	return p;
}

requisit* metadata::find(const char* id) const {
	for(auto& e : requisit_data) {
		if(e.parent==this && strcmp(e.id, id) == 0)
			return &e;
	}
	return 0;
}