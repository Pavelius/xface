#include "collection.h"
#include "crt.h"
#include "requisit.h"

extern "C" void* malloc(unsigned size);
extern "C" void* realloc(void *ptr, unsigned size);
extern "C" void	free(void* pointer);

namespace compiler {
struct archive {
	struct header {
		char		signature[4];
		char		version[4];
		void		set(const char* v) { signature[0] = v[0]; signature[1] = v[1]; signature[2] = v[2]; signature[3] = 0; }
		void		set(int v1, int v2) { version[0] = v1 + 0x30; version[1] = '.'; version[2] = v2 + 0x30; version[3] = 0; }
	};
	struct placement {
		unsigned	offset;
		unsigned	count;
	};
	struct file : header {
		placement	strings;
		placement	requisits;
	};
};
}

using namespace compiler;

static adat<requisit, 8192> elements;

requisit compiler::number[] = {{"number"}};
requisit compiler::pointer[] = {{"pointer"}};
requisit compiler::text[] = {{"string"}};
requisit compiler::object[] = {{"object"}}; // All types get this type parent

static unsigned optimal(unsigned need_count) {
	const unsigned mc = 256 * 256 * 256;
	unsigned m = 64;
	while(m < mc) {
		if(need_count < m)
			return m;
		m = m << 1;
	}
	return m;
}

void* compiler::rmreserve(void* data, unsigned count, unsigned& count_maximum, unsigned size) {
	if(!size)
		return data;
	if(data && count < count_maximum)
		return data;
	count_maximum = optimal(count);
	if(data)
		data = realloc(data, count_maximum*size);
	else
		data = malloc(count_maximum*size);
	return data;
}

array::~array() {
	if(data)
		free(data);
	data = 0;
}

void* array::add() {
	reserve(count + 1);
	auto p = (char*)data + size * count;
	count++;
	return p;
}

unsigned string::add(const char* v) {
	if(!v || v[0]==0)
		return 0xFFFFFFFF;
	if(index.count) {
		for(unsigned i = 0; i < index.count; i++) {
			if(strcmp(data + index.data[i], v) == 0)
				return i;
		}
	}
	index.add(count);
	auto n = zlen(v);
	data = (char*)rmreserve(data, count + n + 1, count_maximum, sizeof(char));
	memcpy(data + count, v, n + 1);
	return index.count - 1;
}

const char* string::get(unsigned value) const {
	return (value < index.count) ? (char*)data + ((unsigned*)index.data)[value] : "";
}

void* requisit::operator new(unsigned size) {
	return elements.add();
}

void requisit::operator delete(void* ptr, unsigned size) {
	memset(ptr, 0, size);
}

void requisit::clear() {
	memset(this, 0, sizeof(requisit));
}

bool requisit::isobject() const {
	for(auto p = parent; p; p = p->parent)
		if(p == object)
			return true;
	return false;
}

requisit* requisit::add(const char* id, requisit* type) {
	auto p = new requisit;
	p->clear();
	p->id = szdup(id);
	p->type = type;
	p->parent = this;
	return p;
}

requisit* requisit::add(const char* id) {
	return object->add(id, 0);
}

requisit* requisit::reference() const {
	for(auto& e : elements) {
		if(e.parent != pointer)
			continue;
		if(e.type == this)
			return &e;
	}
	auto p = new requisit;
	p->clear();
	p->type = const_cast<requisit*>(this);
	p->parent = pointer;
	return p;
}

requisit* requisit::dereference() const {
	if(parent != pointer)
		return 0;
	return type;
}

requisit* manager::create(const char* name) {
	auto p = requisits.add();
	auto id = add(name);
	p->clear();
	p->id = id;
	p->parent = 
}