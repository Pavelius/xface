#include "archive.h"
#include "bsreq.h"
#include "crt.h"
#include "point.h"
#include "setting.h"
#include "stringbuilder.h"
#include "strlib.h"
#include "wizard.h"

#pragma once

struct sprite;

enum map_type_s : unsigned char {
	Rectangle, IsometricRectangle, Hexagon,
};
enum variant_s : unsigned char {
	NoVariant,
	Tileset,
};
enum direction_s : unsigned char {
	Center,
	Left, Up, Right, Down,
};
struct resourcei {
	map_type_s			type;
	char				name[32];
	point				element;
	constexpr explicit operator bool() const { return name[0] != 0; }
};
struct directioni {
	const char*			id;
	const char*			name;
};
class tileset : resourcei {
	sprite*				data;
public:
	tileset();
	static tileset*		add(const char* id);
	static const char*	base_url;
	static const char*	choosenew();
	static int			getcurrentframe();
	const char*			getname() { return name; }
	static const char*	getname(const void* object, stringbuilder& sb);
	const sprite*		getsprite();
	static const char*	geturl(char* buffer, const char* name);
	void				import();
};
struct variant {
	variant_s			type;
	unsigned char		value;
	constexpr variant() : type(NoVariant), value(0) {}
	constexpr variant(variant_s t, unsigned char v) : type(t), value(v) {}
	variant(variant_s t, const array& source, const void* v);
	variant(const tileset* v) : variant(Tileset, bsdata<tileset>::source, v) {}
	constexpr explicit operator bool() const { return type != NoVariant; }
	tileset*			getresource() const;
};
struct map_typei {
	const char*			id;
	const char*			name;
};
struct object : point {
	variant				kind;
	short unsigned		frame;
	short unsigned		flags;
	explicit operator bool() const { return kind.operator bool(); }
	void				draw(point camera) const;
};
struct mapi : resourcei {
	point				size;
	point				screen; // Screen coordinates
	point				offset; // Offset from center in elements
};
struct tileimport {
	char				source[260];
	char				destination[260];
	direction_s			base_x, base_y;
	point				offset;
	tileimport();
	void				execute();
	int					getcenter(int dimension, direction_s link, int value) const;
};
void					add_tileset();
extern tileset*			current_tileset;
void					logmsg(const char* format, ...);