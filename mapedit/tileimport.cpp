#include "main.h"
#include "draw.h"
#include "io.h"

void	sprite_write(const sprite* p, const char* url);
int		sprite_store(sprite* ps, const unsigned char* p, int width, int w, int h, int ox, int oy, sprite::encodes mode = sprite::Auto, unsigned char shadow_index = 0, color* original_pallette = 0, int explicit_frame = -1, unsigned char transparent_index = 0xFF);
void	sprite_create(sprite* p, int count, int cicles, int additional_bytes);

static sprite* create_sprite(unsigned count) {
	auto ps = (sprite*)new char[1024 * 1024 * 16];
	sprite_create(ps, count, 0, 0);
	return ps;
}

static void close_sprite(sprite* p, const char* name) {
	char path[260]; stringbuilder sb(path);
	sb.add("%2/%1.pma", name, tileset::base_url);
	sprite_write(p, path);
	delete p;
}

static unsigned getfilecount(const char* url) {
	unsigned result = 0;
	for(io::file::find f(url); f; f.next()) {
		auto pn = f.name();
		if(pn[0] == '.')
			continue;
		result++;
	}
	return result;
}

static void create_sprite_from_bitmap(const char* name, const char* url) {
	sprite* ps = create_sprite(getfilecount(url));
	for(io::file::find f(url); f; f.next()) {
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

void tileimport::execute() {
	create_sprite_from_bitmap(destination, source);
}