#include "xface/crt.h"
#include "main.h"

using namespace code;

const unsigned pointer_size = 4;
const unsigned array_size = sizeof(arem<char>);

static metadata void_meta = {"Void"};
static metadata text_meta = {"Text", 0, pointer_size};
static metadata int_meta = {"Integer", 0, pointer_size};
static metadata uint_meta = {"Unsigned", 0, pointer_size};
static metadata sint_meta = {"Short", 0, pointer_size / 2};
static metadata usint_meta = {"Short Unsigned", 0, pointer_size / 2};
static metadata char_meta = {"Char", 0, pointer_size / 4};
static metadata type_meta = {"Type"};

static metadata* custom_types[] = {&text_meta, &int_meta, &uint_meta, &sint_meta, &uint_meta, &char_meta, &type_meta};

adat<metadata, 256 * 4>			code::metadata_data;
static adat<metadata, 256 * 8>	pointers;
static adat<metadata, 256 * 8>	arrays;

metadata* findpointer(metadata* m) {
	for(auto& e : pointers) {
		if(e.type == m)
			return &e;
	}
	return 0;
}

metadata* code::findtype(const char* id) {
	for(auto p : custom_types) {
		if(strcmp(p->id, id) == 0)
			return p;
	}
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

bool metadata::isnumber() const {
	return this == &int_meta
		|| this == &uint_meta
		|| this == &sint_meta
		|| this == &usint_meta
		|| this == &char_meta;
}

bool metadata::ismeta() const {
	return this == &type_meta;
}

bool metadata::ispredefined() const {
	for(auto p : custom_types) {
		if(p == this)
			return true;
	}
	return false;
}

bool metadata::istext() const {
	return this == &text_meta;
}

void metadata::initialize() {
	auto p = &type_meta;
	if(p->requisits)
		return;
	p->add("id", "Text");
	p->add("type", "Type*");
	p->add("offset", "Unsigned");
	p->add("count", "Unsigned");
	p->update();
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
	if(!type)
		return 0;
	id = szdup(id);
	auto p = find(id);
	if(!p) {
		p = requisits.add();
		memset(p, 0, sizeof(requisit));
	}
	p->id = id;
	p->type = type;
	p->count = 1;
	return p;
}

const requisit* requisitc::find(const char* id) const {
	for(auto& e : *this) {
		if(strcmp(e.id, id) == 0)
			return &e;
	}
	return 0;
}

unsigned requisit::getsize() const {
	if(type->isreference())
		return pointer_size;
	else if(type->isarray())
		return array_size;
	return type->size;
}

void metadata::update() {
	size = 0;
	for(auto& e : requisits) {
		e.offset = size;
		size += e.getsizeof();
	}
}

bool metadata::is(const char* id) const {
	return this && strcmp(this->id, id) == 0;
}

const requisit* metadata::getid() const {
	if(!requisits.count)
		return 0;
	auto p = requisits.data;
	if(p->type->istext())
		return p;
	return 0;
}

const char* pointer_id = "*";
const char* array_id = "%";

metadata* metadatac::find(const char* id) const {
	for(auto pb = (arraydata*)this; pb; pb = pb->next) {
		auto pe = (element*)pb->end(sizeof(element));
		for(auto p = (element*)pb->begin(); p < pe; p++) {
			if(strcmp(p->id, id) == 0)
				return p;
		}
	}
	return 0;
}

metadata* metadatac::find(const char* id, const metadata* type) const {
	for(auto pb = (arraydata*)this; pb; pb = pb->next) {
		auto pe = (element*)pb->end(sizeof(element));
		for(auto p = (element*)pb->begin(); p < pe; p++) {
			if(p->type != type)
				continue;
			if(strcmp(p->id, id) != 0)
				continue;
			return p;
		}
	}
	return 0;
}

metadata* metadatac::add(const char* id) {
	auto p = find(id);
	if(!p) {
		p = (metadata*)arraydata::add(sizeof(element));
		memset(p, 0, sizeof(element));
		p->id = szdup(id);
	}
	return p;
}

metadata* metadatac::reference(metadata* type) {
	auto p = find(pointer_id, type);
	if(!p) {
		p = (metadata*)arraydata::add(sizeof(element));
		p->id = pointer_id;
		p->type = type;
	}
	return p;
}