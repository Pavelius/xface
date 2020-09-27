#include "main.h"
#include "draw.h"

BSDATAC(tileset, 256);

const char*		tileset::base_url = "sprites";

tileset::tileset() {
	memset(this, 0, sizeof(*this));
}

const sprite* tileset::getsprite() {
	if(!data) {
		char temp[260]; stringbuilder sb(temp);
		sb.add(base_url);
		sb.add("/");
		sb.add(name);
		sb.add(".pma");
		data = (sprite*)loadb(temp);
		if(data) {
			element.x = data->width;
			element.y = data->height;
		}
	}
	return data;
}

const char*	tileset::getname(const void* object, stringbuilder& sb) {
	return ((tileset*)object)->getname();
}

const char*	tileset::geturl(char* buffer, const char* name) {
	stringbuilder sb(buffer, buffer+259);
	sb.add("%1/%2.pma", base_url, name);
	return buffer;
}

tileset* tileset::add(const char* id) {
	for(auto& e : bsdata<tileset>()) {
		if(!e)
			continue;
		if(strcmp(e.name, id) == 0)
			return &e;
	}
	auto p = bsdata<tileset>::add();
	zcpy(p->name, id);
	return p;
}

static setting::element tileset_url[] = {{"Спрайты", {tileset::base_url, setting::Url}},
};
static setting::header headers[] = {{"Рабочий стол", "Пути", 0, tileset_url},
};

void add_tileset() {
	auto p = tileset::choosenew();
	if(!p)
		return;
	tileset::add(p);
}