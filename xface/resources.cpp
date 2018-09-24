#include "crt.h"
#include "draw.h"
#include "io.h"
#include "resources.h"

sprite* gres(int id) {
	if(!id) // First resource is empthy
		return 0;
	auto& e = res::elements[id];
	if(e.notfound)
		return 0;
	if(!e.data) {
		char temp[260];
		e.data = (sprite*)loadb(szurl(temp, e.folder, e.name, "pma"));
		if(!e.data)
			e.notfound = true;
	}
	return e.data;
}

void res::cleanup() {
	for(auto* p = res::elements; p->name; p++) {
		if(p->name) {
			delete p->data;
			p->data = 0;
			p->notfound = false;
		}
	}
}

const char* res::getname(int rid) {
	return elements[rid].name;
}