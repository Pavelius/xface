#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/stringcreator.h"
#include "xface/draw.h"
#include "xface/io.h"
#include "xml.h"
#include "util.h"

void				printcnf(const char* text);
sprite::frame&		sprite_add(sprite* p);
void*				sprite_add(sprite* p, const void* data, int dsize);
void				sprite_create(sprite* p, int count, int cicles, int additional_bytes);
int					sprite_store(sprite* ps, const unsigned char* p, int width, int w, int h, int ox, int oy, sprite::encodes mode, unsigned char shadow_index, color* original_pallette, int explicit_frame, unsigned char transparent_index);
void				sprite_write(const sprite* p, const char* url);
static const char*	tempref[4096 * 2];

static const char* parse_component(const char* p, unsigned char& a) {
	static const char* spearators = " ,\n\r";
	a = sz2num(p, &p);
	while(*p && zchr(spearators, *p))
		p++;
	return p;
}

static void printcf(const char* format, ...) {
	char temp[1024]; szprintvs(temp, zendof(temp), format, xva_start(format));
	printcnf(temp);
}

static void status_loading(const char* type, const char* url) {
	printcf("loading %1 \'%2\'", type, url);
}

static void status_saving(const char* url) {
	printcf("%1\n", url);
}

static void error_not_found(const char* url) {
	printcf("Not found '%1'\n", url);
}

static void error_unknown(const char* name, const char* value) {
	printcf("Unknown %1 '%2'\n", name, value);
}

struct content {
	enum				directions { Left, Right, Up, Down };
	char				input[260]; // input file name
	char				output[260]; // output file name
	char				pal[260];
	int					width, height;
	int					x, y;
	int					ox, oy;
	int					step_x, step_y;
	int					max_x, max_y;
	int					count;
	directions			direction;
	adat<color, 4>		alpha;
	sprite::encodes		mode;

	void decode_alpha(const char* p) {
		alpha.clear();
		while(*p) {
			color m;
			p = parse_component(p, m.r);
			p = parse_component(p, m.g);
			p = parse_component(p, m.b);
			m.a = 0xFF;
			alpha.add(m);
		}
	}

	directions decode_direction(const char* p) {
		if(szcmpi(p, "down") == 0)
			return Down;
		else if(szcmpi(p, "left") == 0)
			return Left;
		else
			return Right;
	}

	// Read parameters for parsing element
	void read(xml::reader& doc) {
		while(doc.type == xml::Attribute) {
			if(doc.attribute == "input") {
				if(doc.attribute.value[0] == '.' && doc.attribute.value[1] == '.' && doc.attribute.value[2] == '.')
					zcat(input, doc.attribute.value + 3);
				else
					zcpy(input, doc.attribute.value);
			} else if(doc.attribute == "output") {
				if(doc.attribute.value[0] == '.' && doc.attribute.value[1] == '.' && doc.attribute.value[2] == '.')
					zcat(output, doc.attribute.value + 3);
				else
					zcpy(output, doc.attribute.value);
			} else if(doc.attribute == "pallette")
				zcpy(pal, doc.attribute.value);
			else if(doc.attribute == "width")
				width = sz2num(doc.attribute.value);
			else if(doc.attribute == "height")
				height = sz2num(doc.attribute.value);
			else if(doc.attribute == "x")
				x = sz2num(doc.attribute.value);
			else if(doc.attribute == "y")
				y = sz2num(doc.attribute.value);
			else if(doc.attribute == "ox")
				ox = sz2num(doc.attribute.value);
			else if(doc.attribute == "oy")
				oy = sz2num(doc.attribute.value);
			else if(doc.attribute == "step_x")
				step_x = sz2num(doc.attribute.value);
			else if(doc.attribute == "step_y")
				step_y = sz2num(doc.attribute.value);
			else if(doc.attribute == "max_x")
				max_x = sz2num(doc.attribute.value);
			else if(doc.attribute == "max_y")
				max_y = sz2num(doc.attribute.value);
			else if(doc.attribute == "count")
				count = sz2num(doc.attribute.value);
			else if(doc.attribute == "mode")
				mode = (sprite::encodes)sz2num(doc.attribute.value);
			else if(doc.attribute == "alpha")
				decode_alpha(doc.attribute.value);
			else if(doc.attribute == "direction")
				direction = decode_direction(doc.attribute.value);
			else
				error_unknown("attribute", doc.attribute.name);
			doc.next();
		}
	}

};

static void apply_alpha(draw::surface& bmp, const content& e) {
	if(!e.alpha.count)
		return;
	int size = bmp.width*bmp.height;
	color* p = (color*)bmp.ptr(0, 0);
	color* pe = p + size;
	while(p < pe) {
		for(unsigned i = 0; i < e.alpha.count; i++) {
			color m = e.alpha.data[i];
			if(p->r == m.r
				&& p->g == m.g
				&& p->b == m.b) {
				p->a = 0;
				break;
			}
		}
		p++;
	}
}

static const char* get(const char* source, const char* file, const char* suffix = 0, const char* ext = 0) {
	static char temp[260];
	return szurl(temp, source, file, ext, suffix);
}

static const char* get_file_name(const char* name) {
	static const char* images[] = {"png", "bmp"};
	for(auto ext : images) {
		const char* url = get(0, name, 0, ext);
		if(io::file::exist(url))
			return url;
	}
	return "";
}

static unsigned char primary[256];
static unsigned char secondary[256];
static unsigned char tertiary[256];
static unsigned char buf1[128 * 128];
static unsigned char buf2[128 * 128];
static short int widths[4096];
static int intervals[][2] = {{0x21, 0x7E},
{0x410, 0x44F},
{0x456, 0x457}, // ”краинские буквы маленькие
{0x406, 0x407}, // ”краинские буквы большие
};

inline int floor(double f) {
	return ((int)(f*10.00)) / 10;
}

static void lcd_init(double prim, double second, double tert, int num_levels) {
	double norm = (255.0 / (num_levels - 1)) / (prim + second * 2 + tert * 2);
	prim *= norm;
	second *= norm;
	tert *= norm;
	for(int i = 0; i < num_levels; i++) {
		primary[i] = floor(prim*i);
		secondary[i] = floor(second*i);
		tertiary[i] = floor(tert*i); // [MODIFY] was floor
	}
}

static void lcd_prepare_glyph(unsigned char* gbuf2, int& w2, const unsigned char* gbuf1, int w1, int h1) {
	int src_stride = (w1 + 3) / 4 * 4;
	int dst_width = src_stride + 4;
	for(int y = 0; y < h1; ++y) {
		const unsigned char* src_ptr = gbuf1 + src_stride * y;
		unsigned char* dst_ptr = gbuf2 + dst_width * y;
		for(int x = 0; x < w1; ++x) {
			unsigned v = *src_ptr++;
			dst_ptr[0] = imin(dst_ptr[0] + tertiary[v], 255);
			dst_ptr[1] = imin(dst_ptr[1] + secondary[v], 255);
			dst_ptr[2] = imin(dst_ptr[2] + primary[v], 255);
			dst_ptr[3] = imin(dst_ptr[3] + secondary[v], 255);
			dst_ptr[4] = imin(dst_ptr[4] + tertiary[v], 255);
			++dst_ptr;
		}
	}
	w2 = dst_width;
}

static void glyph(util::font& font, int g, struct sprite& sprite) {
	int w1 = 0, w2 = 0, dx, dy, ox, oy;
	int h1 = 0;
	memset(buf1, 0, sizeof(buf1));
	memset(buf2, 0, sizeof(buf2));
	int size = font.glyphi(g, w1, h1, dx, dy, ox, oy, buf1, sizeof(buf1), 3, 1);
	if(!size)
		return;
	lcd_prepare_glyph(buf2, w2, buf1, w1, h1);
	sprite_store(&sprite, buf2, w2, ((w1 + 5) / 3) * 3, h1, -(ox + 1) / 3, oy, sprite::ALC,
		0, 0, 0, 0);
	int id = sprite.glyph(g);
	widths[id] = (dx + 2) / 3;
}

static bool image_fnt(xml::reader& doc, const content& parent, struct sprite& sprite) {
	if(doc.type == xml::NameBegin && doc.element == "font") {
		doc.next();
		int height, width;
		int ascend;
		int descend;
		content e = parent; e.read(doc);
		status_loading("fnt", e.input);
		util::font	font(e.input, e.height);
		int glyph_count = 0;
		for(auto& e : intervals)
			glyph_count += e[1] - e[0] + 1;
		sprite_create(&sprite, glyph_count, 0, 0);
		sprite_add(&sprite, intervals, sizeof(intervals));
		for(auto& e : intervals) {
			for(int i = e[0]; i <= e[1]; i++)
				glyph(font, i, sprite);
		}
		sprite_add(&sprite, widths, sizeof(widths[0])*glyph_count);
		font.info(width, height, ascend, descend);
		sprite.height = height;
		sprite.width = draw::textw(&sprite);
		sprite.ascend = ascend;
		sprite.descend = descend;
		doc.next();
		return true;
	}
	return false;
}

static void image(const char* url, const content& e, struct sprite& sprite) {
	draw::surface et;
	et.read(url, 0);
	apply_alpha(et, e);
	if(!et.width || !et.height) {
		error_not_found(url);
		return;
	}
	int ox = e.ox;
	if(ox == -1)
		ox = et.width / 2;
	else if(ox < 0)
		ox += et.width;
	int oy = e.oy;
	if(oy == -1)
		oy = et.height / 2;
	else if(oy < 0)
		oy += et.height;
	int height = e.height;
	if(height == -1)
		height = et.height;
	int width = e.width;
	if(width == -1)
		width = et.width;
	sprite::frame& f = sprite_add(&sprite);
	sprite_store(&sprite, (unsigned char*)et.ptr(e.x, e.y),
		et.scanline,
		width, height, ox, oy, e.mode,
		0,0,0,0);
	switch(f.encode) {
	case sprite::RAW:
		status_loading("raw", url);
		break;
	default:
		status_loading("spr", url);
		break;
	}
}

static int compare(const void* e1, const void* e2) {
	const char* p1 = *((const char**)e1);
	const char* p2 = *((const char**)e2);
	for(; *p2 && *p1; p1++, p2++) {
		if(*p1 == *p2)
			continue;
		if(szupper(*p1) == szupper(*p2))
			continue;
		if((*p1 >= '0' && *p1 <= '9')
			|| (*p2 >= '0' && *p2 <= '9'))
			break;
		return szupper(*p1) - szupper(*p2);
	}
	if(*p1 >= '0' && *p1 <= '9' && *p2 >= '0' && *p2 <= '9') {
		int n1 = sz2num(p1);
		int n2 = sz2num(p2);
		return n1 - n2;
	}
	return szupper(*p1) - szupper(*p2);
}

static bool image_msk(xml::reader& doc, const content& parent, struct sprite& sprite) {
	char temp[260];
	char lurl[260];
	const char** pt = tempref;
	if(doc.type == xml::NameBegin && doc.element == "files") {
		doc.next();
		content e = parent; e.read(doc);
		draw::surface et;
		zcpy(lurl, e.input);
		const char* filter = szfname(e.input);
		((char*)szfname(lurl))[-1] = 0;
		for(io::file::find f(lurl); f; f.next()) {
			if(strcmp(f.name(), ".") == 0)
				continue;
			if(strcmp(f.name(), "..") == 0)
				continue;
			if(!szpmatch(f.name(), filter))
				continue;
			*pt++ = szdup(f.name());
		}
		qsort(tempref, pt - tempref, sizeof(tempref[0]), compare);
		for(const char** p1 = tempref; p1 < pt; p1++) {
			zcpy(temp, lurl);
			zcat(temp, "\\");
			zcat(temp, *p1);
			image(temp, e, sprite);
		}
		doc.next();
		return true;
	}
	return false;
}

static bool image_grd(xml::reader& doc, const content& parent, struct sprite& sprite) {
	if(doc.type == xml::NameBegin && doc.element == "grd") {
		doc.next();
		content e = parent; e.read(doc);
		draw::surface et;
		const char* url = get_file_name(e.input);
		if(!et.read(url, 0, -32))
			error_not_found(url);
		else {
			apply_alpha(et, e);
			if(et.width && et.height && e.width != -1 && e.height != -1) {
				status_loading("grd", url);
				int ox = e.ox;
				if(ox == -1)
					ox = e.width / 2;
				int oy = e.oy;
				if(oy == -1)
					oy = e.height / 2;
				if(!e.step_x)
					e.step_x = e.width;
				if(!e.step_y)
					e.step_y = e.height;
				if(!e.max_x)
					e.max_x = et.width / e.step_x;
				if(!e.max_y)
					e.max_y = et.height / e.step_y;
				switch(e.direction) {
				case content::Down:
					for(int x = 0; x < e.max_x; x++) {
						for(int y = 0; y < e.max_y; y++) {
							sprite_store(&sprite, (unsigned char*)et.ptr(e.x + x * e.step_x, e.y + y * e.step_y),
								et.scanline,
								e.width, e.height, ox, oy, sprite.RLE,
								0, 0, 0, 0);
						}
					}
					break;
				default:
					for(int y = 0; y < e.max_y; y++) {
						for(int x = 0; x < e.max_x; x++) {
							sprite_store(&sprite, (unsigned char*)et.ptr(e.x + x * e.step_x, e.y + y * e.step_y),
								et.scanline,
								e.width, e.height, ox, oy, sprite.RLE,
								0, 0, 0, 0);
						}
					}
					break;
				}
			}
		}
		doc.next();
		return true;
	}
	return false;
}

static bool image_tbr(xml::reader& doc, const content& parent, struct sprite& sprite) {
	if(doc.type == xml::NameBegin && doc.element == "toolbar") {
		doc.next();
		content e = parent; e.read(doc);
		draw::surface et;
		const char* url = get_file_name(e.input);
		int err = et.read(url, 0, -32);
		if(et.width && et.height) {
			status_loading("tbr", url);
			int ox = e.ox;
			if(ox == -1)
				ox = et.height / 2;
			int oy = e.oy;
			if(oy == -1)
				oy = et.height / 2;
			int m = et.width / et.height;
			for(int i = 0; i < m; i++)
				sprite_store(&sprite, (unsigned char*)et.ptr(e.x + i * et.height, e.y),
					et.scanline,
					et.height, et.height, ox, oy, sprite.RLE,
					0, 0, 0, 0);
		} else
			error_not_found(url);
		doc.next();
		return true;
	}
	return false;
}

static bool image_png(xml::reader& doc, const content& parent, struct sprite& sprite) {
	if(doc.type == xml::NameBegin && doc.element == "png") {
		doc.next();
		content e = parent; e.read(doc);
		const char* url = get_file_name(e.input);
		image(url, e, sprite);
		doc.next();
		return true;
	}
	return false;
}

static bool group(xml::reader& doc, const content& parent, struct sprite& sprite) {
	if(doc.type == xml::NameBegin && doc.element == "group") {
		doc.next();
		content e = parent; e.read(doc);
		while(doc.type != xml::NameEnd && doc.type != xml::FileEnd) {
			if(group(doc, e, sprite))
				continue;
			if(image_png(doc, e, sprite))
				continue;
			if(image_tbr(doc, e, sprite))
				continue;
			if(image_grd(doc, e, sprite))
				continue;
			if(image_fnt(doc, e, sprite))
				continue;
			error_unknown("element", doc.element.name);
			doc.skip_block();
		}
		doc.next();
		return true;
	}
	return false;
}

static bool library(xml::reader& doc, const content& parent) {
	if(doc.type == xml::NameBegin && doc.element == "library") {
		doc.next();
		content e = parent; e.read(doc);
		struct sprite* sprite = (struct sprite*)new char[256 * 256 * 256];
		sprite_create(sprite, e.count, 0, 0);
		while(doc.type != xml::NameEnd && doc.type != xml::FileEnd) {
			if(group(doc, e, *sprite))
				continue;
			if(image_msk(doc, e, *sprite))
				continue;
			if(image_png(doc, e, *sprite))
				continue;
			if(image_tbr(doc, e, *sprite))
				continue;
			if(image_grd(doc, e, *sprite))
				continue;
			if(image_fnt(doc, e, *sprite))
				continue;
			error_unknown("element", doc.element.name);
			doc.skip_block();
		}
		status_saving(get(0, e.output, 0, "pma"));
		sprite_write(sprite, get(0, e.output, 0, "pma"));
		delete sprite;
		doc.next();
		return true;
	}
	return false;
}

static bool convert(xml::reader& doc, const content& parent) {
	if(doc.type == xml::NameBegin && doc.element == "convert") {
		doc.next();
		content e = parent; e.read(doc);
		while(doc.type != xml::NameEnd && doc.type != xml::FileEnd) {
			if(library(doc, e))
				continue;
			error_unknown("element", doc.element.name);
			doc.skip_block();
		}
		doc.next();
		return true;
	}
	return false;
}

static int parse(const char* url) {
	content	e;
	xml::reader doc(url);
	if(!doc) {
		error_not_found(url);
		return -1;
	}
	memset(&e, 0, sizeof(e));
	e.width = -1;
	e.height = -1;
	e.ox = -1;
	e.oy = -1;
	e.count = 1;
	e.direction = content::Right;
	if(!convert(doc, e))
		return -1;
	return 0;
}

int main(int argc, char *argv[]) {
	if(argc > 1) {
		lcd_init(0.5, 0.25, 0.125, 65);
		return parse(argv[1]);
	} else {
		printcf("Image library creater 1.0 by Pavel Chistyakov (c)2014\n");
		printcf("Usage: xmlimg <input file.xml>\n");
	}
	return 0;
}