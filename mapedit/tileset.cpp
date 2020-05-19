#include "main.h"
#include "draw.h"
#include "io.h"

INSTDATAC(tileset, 256);

static const char*	url_sprites = "sprites";

void	sprite_write(const sprite* p, const char* url);
int		sprite_store(sprite* ps, const unsigned char* p, int width, int w, int h, int ox, int oy, sprite::encodes mode = sprite::Auto, unsigned char shadow_index = 0, color* original_pallette = 0, int explicit_frame = -1, unsigned char transparent_index = 0xFF);

static sprite* create_sprite() {
	return (sprite*)new char[1024 * 1024 * 16];
}

static sprite* close_sprite(sprite* p, const char* name) {
	char path[260]; stringbuilder sb(path);
	sb.add("art/sprites/%1.pma", name);
	sprite_write(p, path);
	delete p;
}

static void create_sprite_from_bitmap(const char* name, const char* url) {
	char path[260]; stringbuilder sb(path); sb.add("/*.*");
	sprite* ps = create_sprite();
	for(io::file::find f(path); f; f.next()) {
		auto pn = f.name();
		if(pn[0] == '.')
			continue;
		char temp[260];
		draw::surface sf;
		sf.read(f.fullname(temp));
		if(!sf)
			continue;
		sprite_store(ps, sf.ptr(0, 0), sf.scanline,
			sf.width, sf.height,
			sf.width / 2, sf.height / 2);
	}
	close_sprite(ps, name);
}

tileset::tileset() {
	memset(this, 0, sizeof(*this));
}

void tileset::read() {
	if(data)
		delete data;
	char temp[260]; stringbuilder sb(temp);
	sb.add(folder);
	sb.add("/");
	sb.add(name);
	sb.add(".pma");
	data = (sprite*)loadb(temp);
	if(!data)
		return;
}

//void tileset::initialize() {
//	auto& e1 = draw::settings::root.gr("Пути").gr("Общие");
//	e1.add("Спрайты", url_sprites, draw::settings::UrlFolderPtr);
//}

const char*	tileset::geturl(char* buffer, const char* name) {
	stringbuilder sb(buffer, buffer+259);
	sb.add("%1/%2.pma", url_sprites, name);
	return buffer;
}

static setting::element tileset_url[] = {{"Спрайты", {setting::Url, &url_sprites, 0}},
};
static setting::header headers[] = {{"Рабочий стол", "Пути", 0, tileset_url},
};