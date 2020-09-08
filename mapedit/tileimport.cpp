#include "main.h"
#include "draw.h"
#include "io.h"

//void sprite_write(const sprite* p, const char* url);
int	sprite_store(sprite* ps, const unsigned char* p, int width, int w, int h, int ox, int oy, sprite::encodes mode = sprite::Auto, unsigned char shadow_index = 0, color* original_pallette = 0, int explicit_frame = -1, unsigned char transparent_index = 0xFF);
void sprite_create(sprite* p, int count, int cicles, int additional_bytes);
void pma_write(const char* url, const pma** source);

static sprite* create_sprite(unsigned count) {
	auto ps = (sprite*)new char[1024 * 1024 * 16];
	sprite_create(ps, count, 0, 0);
	return ps;
}

static void close_sprite(sprite* p, const char* name, const strlib* strings = 0) {
	if(!p->width) {
		auto c = p->count;
		for(auto i = 0; i < c; i++) {
			auto& f = p->get(i);
			auto rc = f.getrect(0, 0, 0);
			auto w = rc.width();
			auto h = rc.height();
			if(p->width < w)
				p->width = w;
			if(p->height < h)
				p->height = h;
		}
	}
	char path[260]; stringbuilder sb(path);
	sb.add("%2/%1.pma", name, tileset::base_url);
	const pma* source[16];
	auto ps = source;
	*ps++ = p;
	if(strings && strings->getcount()) {
		unsigned count = strings->getcount();
		unsigned size = sizeof(pma) + strings->getsize();
		unsigned size_indecies = sizeof(unsigned)*count;
		unsigned size_text = size - sizeof(pma) - size_indecies;
		auto base = strings->get(0);
		auto p1 = (pma*)new char[size];
		memset(p1, 0, size);
		p1->name[0] = 'S'; p1->name[1] = 'T'; p1->name[2] = 'R';
		p1->count = count;
		p1->size = size;
		auto pi = (unsigned*)((char*)p1 + sizeof(pma));
		auto pt = (char*)p1 + sizeof(pma) + sizeof(unsigned)*count;
		memcpy(pt, strings->get(0), size_text);
		for(unsigned i = 0; i < count; i++)
			pi[i] = strings->getoffset(i) + sizeof(pma) + size_indecies;
		*ps++ = p1;
	}
	*ps++ = 0;
	pma_write(path, source);
	for(auto i = 0; source[i]; i++)
		delete source[i];
}

static unsigned getfilecount(const char* url, strlib* names) {
	unsigned result = 0;
	for(io::file::find f(url); f; f.next()) {
		auto pn = f.name();
		if(pn[0] == '.')
			continue;
		names->add(f.name());
		result++;
	}
	return result;
}

static void add_strings(sprite* ps, const strlib& source) {

}

void create_sprite_from_bitmap(const char* name, const char* url, const tileimport& ti) {
	strlib names;
	sprite* ps = create_sprite(getfilecount(url, &names));
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
			ti.getcenter(sf.width, ti.base_x, ti.offset.x),
			ti.getcenter(sf.height, ti.base_y, ti.offset.y));
	}
	close_sprite(ps, name, &names);
}

int	tileimport::getcenter(int dimension, direction_s link, int value) const {
	switch(link) {
	case Left: case Up:return value;
	case Right: case Down: return dimension - value;
	default: return dimension / 2 + value;
	}
}

void tileimport::execute() {
	create_sprite_from_bitmap(destination, source, *this);
}