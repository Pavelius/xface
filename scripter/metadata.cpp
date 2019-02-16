#include "xface/crt.h"
#include "main.h"

const unsigned pointer_size = 4;

metadata int_meta[] = {"i32"};
metadata sint_meta[] = {"i16"};
metadata usint_meta[] = {"u16"};

static adat<requisit, 256 * 16>	requisits;
static adat<metadata, 256 * 4>	types;
static adat<metadata, 256 * 8>	pointers;

metadata* findpointer(metadata* m) {
	for(auto& e : pointers) {
		if(e.type==m)
			return &e;
	}
	return 0;
}

metadata* findtype(const char* id) {
	for(auto& e : types) {
		if(strcmp(e.id, id) == 0)
			return &e;
	}
	return 0;
}

metadata* addtype(const char* id) {
	auto p = findtype(id);
	if(!p)
		p = types.add();
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
	auto p = find(id);
	if(!p)
		p = requisits.add();
	p->id = id;
	p->parent = this;
	p->type = type;
	p->count = 1;
	return p;
}

requisit* metadata::find(const char* id) const {
	for(auto& e : requisits) {
		if(strcmp(e.id, id) == 0)
			return &e;
	}
	return 0;
}