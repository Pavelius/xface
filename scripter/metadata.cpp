#include "xface/crt.h"
#include "main.h"

using namespace code;

INSTMETA(metadata) = {BSREQ(id),
BSREQ(type),
BSREQ(size),
{}};
INSTDATAC(metadata, 2048)

INSTMETA(requisit) = {BSREQ(id),
BSREQ(type),
BSREQ(count),
BSREQ(offset),
{}};
INSTDATAC(requisit, 256*128)

const unsigned	pointer_size = 4;
const unsigned	array_size = sizeof(arem<char>);
const char*		pointer_id = "*";
const char*		array_id = "%";

static void add_standart(const char* id, unsigned size, const cflags<metatype_s>& mf) {
	auto p = addtype(id);
	p->size = size;
	p->flags = mf;
}

void code::initialize() {
	add_standart("Void", 0, {Predefined}); // Must be first metadata
	add_standart("Char", pointer_size / 4, {ScalarType, Predefined});
	add_standart("Byte", pointer_size / 4, {ScalarType, Predefined});
	add_standart("Short", pointer_size/2, {ScalarType, Predefined});
	add_standart("Short Unsigned", pointer_size/2, {ScalarType, Predefined});
	add_standart("Integer", pointer_size, {ScalarType, Predefined});
	add_standart("Unsigned", pointer_size, {ScalarType, Predefined});
	add_standart("Text", pointer_size, {TextType, Predefined});
	add_standart("Type", pointer_size, {Predefined});
	add_standart("Requisit", pointer_size, {Predefined});
	auto p = addtype("Requisit");
	p->add("type", addtype("*Type"));
	p->add("offset", addtype("Unsigned"));
	p->add("count", addtype("Unsigned"));
	p->add("expression", addtype("Unsigned"));
	p->update();
	p = addtype("Type");
	p->add("id", addtype("Text"));
	p->add("type", addtype("*Type"));
	p->add("size", addtype("Unsigned"));
	p->add("flags", addtype("Unsigned"));
	p->update();
}

metadata* code::findtype(const char* id) {
	for(auto& e : bsdata<metadata>()) {
		if(e.is(id))
			return &e;
	}
	return 0;
}

metadata* code::addtype(const char* id) {
	if(id[0] == pointer_id[0])
		return addtype(id + 1)->reference();
	if(id[0] == array_id[0])
		return addtype(id + 1)->array();
	auto p = findtype(id);
	if(p)
		return p;
	p = bsdata<metadata>::add();
	p->id = id;
	return p;
}

requisit* metadata::add(const char* id, metadata* type) {
	if(!type)
		return 0;
	id = szdup(id);
	auto p = find(id);
	if(!p) {
		p = bsdata<requisit>::add();
		memset(p, 0, sizeof(requisit));
		p->parent = const_cast<metadata*>(this);
	}
	p->id = id;
	p->type = type;
	p->count = 1;
	return p;
}

unsigned requisit::getsize() const {
	return type->size;
}

void metadata::update() {
	size = 0;
	for(auto& e : bsdata<requisit>()) {
		if(e.parent != this)
			continue;
		e.offset = size;
		size += e.getsizeof();
	}
}

bool metadata::is(const char* id) const {
	return this && strcmp(this->id, id) == 0;
}

const metadata* metadata::gettype() const {
	if(isarray())
		return type->gettype();
	if(isreference())
		return type->gettype();
	return this;
}

metadata* code::addtype(const char* id, const metadata* type, unsigned size) {
	for(auto& e : bsdata<metadata>()) {
		if(e && e.type == type && e.is(id))
			return &e;
	}
	auto p = bsdata<metadata>::add();
	p->id = szdup(id);
	p->size = size;
	p->type = const_cast<metadata*>(type);
	return p;
}

metadata* metadata::reference() const {
	return addtype(pointer_id, this, pointer_size);
}

metadata* metadata::array() const {
	return addtype(array_id, this, array_size);
}

requisit* metadata::find(const char* id) const {
	for(auto& e : bsdata<requisit>()) {
		if(!e || e.parent != this)
			continue;
		if(strcmp(e.id, id) == 0)
			return &e;
	}
	return 0;
}

void requisit::getname(stringbuilder& sb) const {
	sb.add(id);
	sb.add(":");
	for(auto t = type; t; t = t->type)
		sb.add(t->id);
	if(count > 1)
		sb.add("[%1i]", count);
}