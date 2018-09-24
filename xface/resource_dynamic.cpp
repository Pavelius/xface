#include "collection.h"
#include "crt.h"
#include "draw.h"
#include "resources.h"

using namespace draw;

static arem<res::element> source;

static res::element* find(const char* name, const char* folder) {
	for(auto& e : source) {
		if(e.name == name && e.folder == folder)
			return &e;
	}
	return 0;
}

const sprite* draw::gres(const char* name, const char* folder) {
	auto p = find(name, folder);
	if(!p) {
		p = source.add();
		p->name = name;
		p->folder = folder;
		p->data = 0;
		p->notfound = false;
	}
	if(!p || p->notfound)
		return 0;
	if(!p->data) {
		char temp[260];
		p->data = (sprite*)loadb(szurl(temp, p->folder, p->name, "pma"));
		if(!p->data)
			p->notfound = true;
	}
	return p->data;
}