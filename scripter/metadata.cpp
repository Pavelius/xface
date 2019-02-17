#include "xface/crt.h"
#include "main.h"

using namespace code;

const unsigned pointer_size = 4;

metadata code::void_meta[] = {""};
metadata code::text_meta[] = {"Text"};
metadata code::int_meta[] = {"Integer"};
metadata code::sint_meta[] = {"Short"};
metadata code::usint_meta[] = {"Short Unsigned"};
metadata code::char_meta[] = {"Char"};

adat<requisit, 256 * 16>		code::requisit_data;
adat<metadata, 256 * 4>			code::metadata_data;
static agrw<expression, 256 * 4 * 16> expression_base;
static adat<metadata, 256 * 8>	pointers;

void* expression::operator new(unsigned size) {
	return expression_base.add();
}

void expression::operator delete(void* p, unsigned size) {
}

metadata* findpointer(metadata* m) {
	for(auto& e : pointers) {
		if(e.type==m)
			return &e;
	}
	return 0;
}

metadata* code::findtype(const char* id) {
	for(auto& e : metadata_data) {
		if(strcmp(e.id, id) == 0)
			return &e;
	}
	return 0;
}

metadata* code::addtype(const char* id) {
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