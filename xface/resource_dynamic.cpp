#include "crt.h"
#include "draw.h"
#include "io.h"

using namespace draw;

namespace {
struct resei {
	const char*		name;
	const char*		folder;
	sprite*			data;
	bool			notfound;
};
}

static vector<resei> source;

static resei* find(const char* name, const char* folder) {
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
					unsigned size = sizeof(sprite) + dc.width * dc.height * 3;
					p->data = (sprite*)new char[size]; memset(p->data, 0, size);
					p->data->frames[0].encode = sprite::RAW;
					p->data->frames[0].sx = dc.width;
					p->data->frames[0].sy = dc.height;
					p->data->frames[0].offset = sizeof(sprite);
					p->data->count = 1;
					// Дешевый и простой алгоритм сжатия без прозрачности
					auto pd = (unsigned char*)p->data->ptr(p->data->frames[0].offset);
					for(auto y = 0; y < dc.height; y++) {
						for(auto x = 0; x < dc.width; x++) {
							auto input = dc.ptr(x, y);
							pd[0] = input[0];
							pd[1] = input[1];
							pd[2] = input[2];
							pd += 3;
						}
					}
					break;
				}
			}
		}
	}
	if(!p->data)
		p->notfound = true;
	return p->data;
}