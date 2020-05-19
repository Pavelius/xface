#include "setting2.h"

using namespace setting;

int reference::get() const {
	if(!data)
		return 0;
	switch(type) {
	case Text: case Url:
		return (int)((const char**)data);
	default:
		switch(size) {
		case sizeof(char) : return *((char*)data);
		case sizeof(short) : return *((short*)data);
		default: return *((int*)data);
		}
	}
}

void reference::set(int v) const {
	switch(type) {
	case Text: case Url:
		*((const char**)data) = (const char*)v;
		break;
	default:
		switch(size) {
		case sizeof(char) : *((char*)data) = v; break;
		case sizeof(short) : *((short*)data) = v; break;
		default: *((int*)data) = v; break;
		}
	}
}

header* header::first;

header::header(const char* division, const char* page, const char* group, const list<element>& elements, ptest visible) : division(division), page(page), group(group),
	elements(elements), visible(visible) {
	if(!first)
		first = this;
	else {
		auto p = first;
		while(p->next)
			p = p->next;
		p->next = this;
	}
}