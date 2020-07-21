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

const unsigned			pointer_size = 4;
const unsigned			array_size = sizeof(arem<char>);
const metadata*			metadata::type_metadata;
const metadata*			metadata::type_requisit;
const metadata*			metadata::type_text;
const char*				pointer_id = "*";
const char*				array_id = "&";
const char*				elements_id = "Elements";
const char*				text_id = "Text";
const char*				metadata_id = "Metadata";
const char*				metadata_ptr_id = "*Metadata";
const char*				unsigned_int_id = "UInteger";
static const requisit*	requisit_data;

static const metadata* add_standart(const char* id, unsigned size, const cflags<metatype_s>& mf) {
	auto p = addtype(id);
	p->size = size;
	p->flags = mf;
	return p;
}

void code::initialize() {
	bsdata<metadata>::source.clear();
	bsdata<requisit>::source.clear();
	add_standart("Void", 0, {}); // Must be first metadata
	add_standart("Char", pointer_size / 4, {ScalarType});
	add_standart("UChar", pointer_size / 4, {ScalarType});
	add_standart("Short", pointer_size/2, {ScalarType});
	add_standart("UShort", pointer_size/2, {ScalarType});
	add_standart("Integer", pointer_size, {ScalarType});
	add_standart(unsigned_int_id, pointer_size, {ScalarType});
	metadata::type_text = add_standart(text_id, pointer_size, {});
	metadata::type_metadata = add_standart(metadata_id, sizeof(metadata), {});
	add_standart(metadata_ptr_id, pointer_size, {});
	metadata::type_requisit = add_standart("Requisit", sizeof(requisit), {});
	auto p = addtype("Requisit");
	p->add("id", addtype(text_id))->add(Dimension);
	p->add("type", addtype(metadata_ptr_id));
	p->add("offset", addtype(unsigned_int_id));
	p->add("count", addtype(unsigned_int_id));
	p->add("parent", addtype(metadata_ptr_id))->add(Dimension);
	p->add("flags", addtype(unsigned_int_id));
	p->add(bsdata<requisit>::source_ptr);
	p->update();
	p = addtype(metadata_id);
	p->add("id", addtype(text_id))->add(Dimension);
	p->add("type", addtype(metadata_ptr_id))->add(Dimension);
	p->add("size", addtype(unsigned_int_id));
	p->add("flags", addtype(unsigned_int_id));
	requisit_data = p->add(bsdata<metadata>::source_ptr);
	p->update();
}

int	metadata::getid() const {
	return this - bsdata<metadata>::elements;
}

bool metadata::istext() const {
	return this == type_text;
}

bool metadata::isarray() const {
	return id[0] == array_id[0];
}

bool metadata::isreference() const {
	return id[0] == pointer_id[0];
}

bool metadata::ispredefined() const {
	return this <= type_requisit;
}

void metadata::add(stringbuilder& sb) const {
	for(auto t = this; t; t = t->type)
		sb.add(t->id);
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
	p->id = szdup(id);
	return p;
}

requisit* metadata::add(::array* ptr) {
	auto p = add(elements_id, this->array());
	p->add(Static);
	p->offset = (unsigned)ptr;
	return p;
}

array* metadata::getelements() const {
	auto p = find(elements_id);
	if(!p)
		return 0;
	if(!p->is(Static) || !p->type->isarray() || p->type->type != this)
		return 0;
	return (::array*)p->offset;
}

requisit* metadata::add(const char* id, const metadata* type) {
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
	p->type = const_cast<metadata*>(type);
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
		if(e.is(Static))
			continue;
		e.offset = size;
		size += e.getsizeof();
	}
}

bool metadata::is(const char* id) const {
	return this && strcmp(this->id, id) == 0;
}

const metadata* metadata::gettype() const {
	if(isarray() || isreference())
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

void metadata::getname(stringbuilder& sb) const {
	if(isreference()) {
		sb.add(pointer_id);
		type->getname(sb);
	} else if(isarray()) {
		type->getname(sb);
		sb.add(array_id);
	} else
		sb.add(id);
}

void requisit::getnameonly(stringbuilder& sb) const {
	sb.add(id);
}

void requisit::getname(stringbuilder& sb) const {
	sb.add(id);
	sb.add(":");
	type->getname(sb);
	if(count > 1)
		sb.add("[%1i]", count);
}

bool requisit::ispredefined() const {
	return this <= requisit_data;
}