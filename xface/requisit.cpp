#include "archive.h"
#include "bsreq.h"
#include "crt.h"
#include "requisit.h"
#include "strlib.h"

using namespace compiler;

namespace compiler {
const unsigned	pointer_size = sizeof(void*);
struct serial {
	char			signature[4];
	void set(const char* v) {
		signature[0] = v[0];
		signature[1] = v[1];
		signature[2] = v[2];
		signature[3] = 0;
	}
};
}

unsigned manager::get(const char* v) {
	if(!v || v[0] == 0)
		return Null;
	for(unsigned i = 0; i < strings.count; i++) {
		if(strcmp(section_strings.data + strings.data[i], v) == 0)
			return i;
	}
	auto result = strings.count;
	strings.add(section_strings.count);
	auto n = zlen(v);
	section_strings.reserve(section_strings.count + n + 1);
	memcpy(section_strings.data + section_strings.count, v, n + 1);
	section_strings.count += n + 1;
	return result;
}

unsigned manager::getsize(unsigned v) const {
	return requisits.data[v].size;
}

unsigned manager::create(const char* name) {
	auto p = classes.add();
	p->id = get(name);
	return p - classes.data;
}

unsigned manager::add(unsigned parent, const char* name, unsigned type, unsigned count, unsigned size) {
	auto p = requisits.add();
	p->id = get(name);
	p->parent = parent;
	p->type = type;
	p->count = count;
	p->offset = 0;
	if(!size)
		size = getsize(type);
	p->size = size;
	if(!ispredefined(parent)) {
		p->offset = getsize(parent);
		requisits.data[parent].size += p->getlenght();
	}
	return p - requisits.data;
}

unsigned manager::reference(unsigned v) {
	for(unsigned i = 0; i < requisits.count; i++) {
		if(requisits.data[i].parent != Pointer)
			continue;
		if(requisits.data[i].type == v)
			return i;
	}
	auto p = requisits.data + (requisits.count++);
	p->id = Null;
	p->parent = Pointer;
	p->type = v;
	p->count = 1;
	p->size = pointer_size;
	p->offset = 0;
	return p - requisits.data;
}

bool manager::isreference(unsigned v) const {
	return !ispredefined(v) && requisits.data[v].parent == Pointer;
}

unsigned manager::dereference(unsigned v) const {
	if(!isreference(v))
		return Null;
	return requisits.data[v].type;
}