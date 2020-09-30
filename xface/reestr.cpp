#include "reestr.h"

unsigned reestr::stridx::find(const void* p, unsigned c) {
	auto pb = (char*)begin();
	if(!pb)
		return 0xFFFFFFFF;
	auto pe = (char*)end() - c;
	while(pb < pe) {
		if(memcmp(pb, p, c) == 0)
			return pb - begin();
		pb++;
	}
	return 0xFFFFFFFF;
}

const char* reestr::stridx::get(unsigned id) const {
	if(id >= count)
		return "";
	return (char*)data + id;
}

unsigned reestr::stridx::add(const char* id) {
	if(!id || id[0] == 0)
		return 0xFFFFFFFF;
	auto c = zlen(id) + 1;
	auto i = find(id, c);
	if(i != 0xFFFFFFFF)
		return i;
	i = count;
	reserve(i + c);
	memcpy(ptr(i), id, c);
	return i;
}

reestr::element* reestr::find(unsigned parent, unsigned id) const {
	auto pe = (element*)elements.end();
	for(auto pb = (element*)elements.begin(); pb < pe; pb++) {
		if(pb->parent == parent && pb->id == id)
			return pb;
	}
	return 0;
}

unsigned reestr::next(unsigned parent, unsigned i) const {
	if(i == 0xFFFFFFFF)
		i = 0;
	else
		i++;
	auto pe = (element*)elements.end();
	for(auto pb = (element*)elements.ptr(i); pb < pe; pb++) {
		if(pb->parent != parent)
			continue;
		return pb - (element*)elements.begin();
	}
	return 0xFFFFFFFF;
}

unsigned reestr::add(unsigned parent, const char* id, reestr::type_s type, unsigned value) {
	auto s = strings.add(id);
	auto p = find(parent, s);
	if(!p) {
		p = (element*)elements.add();
		p->parent = parent;
		p->id = s;
	}
	p->type = type;
	p->value = value;
	return p - (element*)elements.begin();
}