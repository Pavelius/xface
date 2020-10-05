#include "main.h"

using namespace code;

BSDATAC(metadata, 2048)
BSDATAC(requisit, 256 * 128)

const unsigned			pointer_size = 4;
const unsigned			array_size = sizeof(vector<char>);
const metadata*			metadata::type_void;
const metadata*			metadata::type_i8;
const metadata*			metadata::type_u8;
const metadata*			metadata::type_i16;
const metadata*			metadata::type_u16;
const metadata*			metadata::type_i32;
const metadata*			metadata::type_u32;
const metadata*			metadata::type_text;
const metadata*			metadata::type_sizet;
const metadata*			metadata::type_metadata;
const metadata*			metadata::type_metadata_ptr;
const metadata*			metadata::type_metadata_array;
const metadata*			metadata::type_requisit;
const metadata*			metadata::type_import;
const metadata*			metadata::type_project;
const char*				metadata::classes_url;
const char*				metadata::projects_url;
const char*				pointer_id = "*";
const char*				array_id = "&";
const char*				elements_id = "Elements";
static const requisit*	last_standart_requisit;

static const metadata* add_standart(const char* id, unsigned size, const cflags<metatype_s>& mf) {
	auto p = addtype(id);
	p->size = size;
	p->flags = mf;
	return p;
}

void code::initialize() {
	bsdata<metadata>::source.clear();
	bsdata<requisit>::source.clear();
	metadata::type_void = add_standart("void", 0, {}); // Must be first metadata
	metadata::type_i8 = add_standart("i8", pointer_size / 4, {ScalarType});
	metadata::type_u8 = add_standart("u8", pointer_size / 4, {ScalarType});
	metadata::type_i16 = add_standart("i16", pointer_size / 2, {ScalarType});
	metadata::type_u16 = add_standart("u16", pointer_size / 2, {ScalarType});
	metadata::type_i32 = add_standart("i32", pointer_size, {ScalarType});
	metadata::type_u32 = add_standart("u32", pointer_size, {ScalarType});
	metadata::type_sizet = metadata::type_u32;
	metadata::type_text = add_standart("Text", pointer_size, {});
	metadata::type_metadata = add_standart("Metadata", sizeof(metadata), {});
	metadata::type_metadata_ptr = metadata::type_metadata->reference();
	metadata::type_metadata_array = metadata::type_metadata->records();
	metadata::type_requisit = add_standart("Requisit", sizeof(requisit), {});
	metadata::type_import = add_standart("import", sizeof(importi), {});
	metadata::type_project = add_standart("Project", sizeof(project), {});
	auto p = const_cast<metadata*>(metadata::type_requisit);
	p->add("id", metadata::type_text)->add(Dimension);
	p->add("Type", metadata::type_metadata_ptr);
	p->add("Offset", metadata::type_sizet);
	p->add("Count", metadata::type_sizet);
	p->add("Parent", metadata::type_metadata_ptr)->add(Dimension);
	p->add("Flags", metadata::type_u32);
	p->add(bsdata<requisit>::source_ptr);
	p->update();
	p = const_cast<metadata*>(metadata::type_metadata);
	p->add("id", metadata::type_text)->add(Dimension);
	p->add("Type", metadata::type_metadata_ptr)->add(Dimension);
	p->add("Count", metadata::type_sizet)->add(Dimension);
	p->add("Size", metadata::type_sizet);
	p->add("Flags", metadata::type_u32);
	p->add(bsdata<metadata>::source_ptr);
	p->update();
	p = const_cast<metadata*>(metadata::type_import);
	p->add("parent", metadata::type_metadata)->add(Dimension);
	p->add("url", metadata::type_text)->add(Dimension);
	last_standart_requisit = p->add(bsdata<importi>::source_ptr);
	p->update();
	//p = const_cast<metadata*>(metadata::type_project);
	//p->add("Name", metadata::type_text);
	//p->add("Description", metadata::type_text);
	//p->add("Modules", metadata::type_text->records());
	//p->update();
}

int	metadata::getid() const {
	return this - bsdata<metadata>::elements;
}

bool metadata::istext() const {
	return this == type_text;
}

bool metadata::isbasetype(const void* object) {
	auto p = (metadata*)object;
	if(p->is(Static))
		return false;
	return true;
}

bool metadata::isarray() const {
	return id[0] == array_id[0];
}

bool metadata::isreference() const {
	return id[0] == pointer_id[0];
}

bool metadata::ispredefined() const {
	return this <= type_project;
}

void metadata::add(stringbuilder& sb, char sep) const {
	if(!this)
		return;
	sb.add(id);
	for(auto t = type; t; t = t->type) {
		if(sep)
			sb.add(sep);
		sb.add(t->id);
	}
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
		return addtype(id + 1)->records();
	auto p = findtype(id);
	if(p)
		return p;
	p = bsdata<metadata>::add();
	p->id = szdup(id);
	p->count = 1;
	return p;
}

requisit* metadata::addm(const char* id, const metadata* type) {
	return add(id, type)->set(Method);
}

requisit* metadata::add(::array* ptr) {
	auto p = add(elements_id, this->records());
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
	p->flags = flags;
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

metadata* code::addtype(const char* id, const metadata* type, unsigned size, unsigned count) {
	for(auto& e : bsdata<metadata>()) {
		if(e && e.type == type  && e.count == count && e.is(id))
			return &e;
	}
	auto p = bsdata<metadata>::add();
	p->id = szdup(id);
	p->size = size;
	p->count = count;
	p->type = const_cast<metadata*>(type);
	return p;
}

metadata* metadata::reference() const {
	return addtype(pointer_id, this, pointer_size);
}

metadata* metadata::records() const {
	return addtype(array_id, this, array_size);
}

const metadata*	metadata::find(const metadata& e1) {
	for(auto& e : bsdata<metadata>()) {
		if(!e)
			continue;
		if(e.type == e1.type && (strcmp(e.id, e1.id) == 0))
			return &e;
	}
	return 0;
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
		type->getname(sb);
		sb.add(pointer_id);
	} else if(isarray()) {
		type->getname(sb);
		sb.add(array_id);
	} else
		sb.add(id);
}

void requisit::getnameonly(stringbuilder& sb) const {
	sb.add(id);
	if(is(Method))
		sb.add("()");
}

void requisit::getname(stringbuilder& sb) const {
	sb.add(id);
	if(is(Method))
		sb.add("()");
	sb.add(":");
	type->getname(sb);
	if(count > 1)
		sb.add("[%1i]", count);
}

void requisit::clear() {
	memset(this, 0, sizeof(*this));
}

bool requisit::ispredefined() const {
	return this <= last_standart_requisit;
}

void requisit::geturl(stringbuilder& sb) const {
	if(!parent)
		return;
	sb.add("req://");
	parent->add(sb, '.');
	sb.add("/");
	sb.add(id);
}