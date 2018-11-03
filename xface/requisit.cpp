#include "collection.h"
#include "crt.h"
#include "requisit.h"

using namespace compiler;

static adat<requisit, 8192> elements;

requisit compiler::number[] = {{"number"}};
requisit compiler::pointer[] = {{"pointer"}};
requisit compiler::text[] = {{"string"}};
requisit compiler::object[] = {{"object"}}; // All types get this type parent

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
		if(e.parent!=pointer)
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