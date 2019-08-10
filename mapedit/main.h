#include "xface/bsreq.h"
#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/point.h"
#include "xface/stringbuilder.h"

#pragma once

enum map_type_s : unsigned char {
	Rectangle, IsometricRectangle,
};

struct map_typei {
	const char*			id;
	const char*			name;
};
struct answer : stringbuilder {
	struct element {
		int				param;
		const char*		text;
		const char*		getname() const { return text; }
	};
	typedef void(*tips_proc)(stringbuilder& sb, const element& e);
	adat<element, 8>	elements;
	constexpr explicit operator bool() const { return elements.count != 0; }
	answer();
	void				add(int param, const char* format, ...);
	void				addv(int param, int priority, const char* format, const char* format_param);
	int					choosev(bool cancel_button, tips_proc tips, const char* format) const;
	void				sort();
private:
	char				buffer[4096];
};
struct mapi {
	map_type_s			type;
	point				element;
	point				size;
	point				screen; // Screen coordinates
	point				offset; // Offset from center in elements
};
struct tileset {
	void				import();
};
DECLENUM(map_type);