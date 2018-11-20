#include "archive.h"
#include "collection.h"
#include "crt.h"
#include "requisit.h"

namespace compiler {
//struct archive {
//	struct header {
//		char		signature[4];
//		char		version[4];
//		void		set(const char* v) { signature[0] = v[0]; signature[1] = v[1]; signature[2] = v[2]; signature[3] = 0; }
//		void		set(int v1, int v2) { version[0] = v1 + 0x30; version[1] = '.'; version[2] = v2 + 0x30; version[3] = 0; }
//	};
//	struct placement {
//		unsigned	offset;
//		unsigned	count;
//	};
//	struct file : header {
//		placement	strings;
//		placement	requisits;
//	};
//};
}

using namespace compiler;

requisit compiler::number[1];
requisit compiler::pointer[1];
requisit compiler::text[1];
requisit compiler::object[1]; // All types get this type parent

unsigned string::add(const char* v) {
	if(!v || v[0]==0)
		return 0xFFFFFFFF;
	if(index.count) {
		for(unsigned i = 0; i < index.count; i++) {
			if(strcmp(data.data + index.data[i], v) == 0)
				return i;
		}
	}
	index.add(data.count);
	auto n = zlen(v);
	data.reserve(data.count + n + 1);
	memcpy(data.data + data.count, v, n + 1);
	data.count += n + 1;
	return index.count - 1;
}

const char* string::get(unsigned value) const {
	return (value < index.count) ? (char*)data.data + ((unsigned*)index.data)[value] : "";
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

requisit* manager::create(const char* name) {
	auto p = requisits.add();
	auto id = strings.add(name);
	p->clear();
	p->id = id;
	p->parent = object;
	return p;
}

requisit* manager::add(requisit* parent, const char* name, requisit* type) {
	auto p = requisits.add();
	auto id = strings.add(name);
	p->clear();
	p->id = id;
	p->parent = parent;
	p->type = type;
	return p;
}

requisit* manager::reference(const requisit* req) {
	for(auto& e : requisits) {
		if(e.parent != pointer)
			continue;
		if(e.type == req)
			return &e;
	}
	auto p = new requisit;
	p->clear();
	p->type = const_cast<requisit*>(req);
	p->parent = pointer;
	return p;
}

requisit* requisit::dereference() const {
	if(parent != pointer)
		return 0;
	return type;
}

template<> void archive::set<string>(string& e) {
	set(e.index);
	set(e.data);
}

void manager::write(const char* url) {
	io::file file(url, StreamWrite);
	if(!file)
		return;
	archive e(file, true);
	e.set(strings);
}