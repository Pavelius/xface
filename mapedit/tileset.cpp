#include "main.h"
#include "draw.h"

INSTDATAC(tileset, 256);

const char*	tileset::base_url = "sprites";

tileset::tileset() {
	memset(this, 0, sizeof(*this));
}

void tileset::read() {
	if(data)
		delete data;
	char temp[260]; stringbuilder sb(temp);
	sb.add(base_url);
	sb.add("/");
	sb.add(name);
	sb.add(".pma");
	data = (sprite*)loadb(temp);
	if(!data)
		return;
}

const char*	tileset::geturl(char* buffer, const char* name) {
	stringbuilder sb(buffer, buffer+259);
	sb.add("%1/%2.pma", base_url, name);
	return buffer;
}

static setting::element tileset_url[] = {{"Спрайты", {tileset::base_url, setting::Url}},
};
static setting::header headers[] = {{"Рабочий стол", "Пути", 0, tileset_url},
};