#include "collection.h"
#include "crt.h"
#include "draw.h"
#include "io.h"
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
		if(!p->data) {
			draw::surface dc;
			for(auto pg = surface::plugin::first; pg; pg = pg->next) {
				szurl(temp, p->folder, p->name, pg->name);
				if(dc.read(temp, 0, 32)) {
					unsigned size = sizeof(sprite) + dc.width*dc.height * sizeof(color);
					p->data = (sprite*)new char[size]; memset(p->data, 0, size);
					p->data->frames[0].encode = sprite::RAW;
					p->data->frames[0].sx = dc.width;
					p->data->frames[0].sy = dc.height;
					p->data->frames[0].offset = sizeof(sprite);
					p->data->count = 1;
					memcpy((void*)p->data->offs(p->data->frames[0].offset), dc.ptr(0, 0), dc.width*dc.height * sizeof(color));
					break;
				}
			}
		}
	}
	if(!p->data)
		p->notfound = true;
	return p->data;
}