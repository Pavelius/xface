#include "bsreq.h"
#include "crt.h"
#include "point.h"
#include "setting.h"
#include "stringbuilder.h"
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
struct resourcei {
	char				name[16];
	char				folder[16];
	map_type_s			type;
	point				element;
};
struct tileset : resourcei {
	sprite*				data;
	void				read();
	tileset();
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
struct answer : stringbuilder {
	struct element {
		int				param;
		const char*		text;
		const char*		getname() const { return text; }
	};
	typedef void(*tips_proc)(stringbuilder& sb, const element& e);
	adat<element, 8>	elements;
	explicit operator bool() const { return elements.count != 0; }
	answer();
	void				add(int param, const char* format, ...);
	void				addv(int param, int priority, const char* format, const char* format_param);
	int					choosev(bool cancel_button, tips_proc tips, const char* format) const;
	void				sort();
private:
	char				buffer[4096];
};
struct mapi : resourcei {
	point				size;
	point				screen; // Screen coordinates
	point				offset; // Offset from center in elements
};
struct tileimport {
	char				source[260];
	char				destination[260];
	tileimport();
};
extern tileset*			current_tileset;