#include "main.h"
#include "draw.h"

INSTDATAC(tileset, 256);

const char*	tileset::base_url = "sprites";

tileset::tileset() {
	memset(this, 0, sizeof(*this));
}

void tileset::read() {
	if(!data) {
		char temp[260]; stringbuilder sb(temp);
		sb.add(base_url);
		sb.add("/");
		sb.add(name);
		sb.add(".pma");
		data = (sprite*)loadb(temp);
	}
}

const char*	tileset::getname(const void* object, char* buffer, const char* buffermax) {
	return ((tileset*)object)->name;
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

static setting::element tileset_url[] = {{"�������", {tileset::base_url, setting::Url}},
};
static setting::header headers[] = {{"������� ����", "����", 0, tileset_url},
};