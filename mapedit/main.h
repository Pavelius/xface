#include "xface/point.h"
#include "xface/bsdata.h"
#include "xface/bsreq.h"
#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/stringcreator.h"

#pragma once

enum map_s : unsigned char {
	Rectangle
};

struct answer : stringcreator {
	struct element {
		int				param;
		const char*		text;
		const char*		getname() const { return text; }
	};
	typedef void(*tips_proc)(stringcreator& sb, const element& e);
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
struct map_info {
	map_s				type;
	point				element;
	point				size;
};
struct tileset {
	void				import();
};