#include "crt.h"
#include "draw_properties.h"
#include "stringbuilder.h"

using namespace draw;
using namespace draw::controls;

int properties::group(int x, int y, int width, void* object, const markup* type) {
	if(!object)
		return 0;
	int y0 = y;
	for(auto p = type; *p; p++)
		y += element(x, y, width, object, p);
	return y - y0;
}

const char* properties::gettitle(stringbuilder& sb, const void* object, const markup* type) const {
	return type->title;
}

void properties::focusfirst() {
	const anyval st(type->value.ptr(object), type->value.size, 0);
	draw::setfocus(st, true);
}

int properties::element(int x, int y, int width, void* object, const markup* type) {
	if(!isvisible(object, type))
		return 0;
	int h = 0;
	char temp[260]; stringbuilder sb(temp);
	if(type->value.isnum()) {
		const anyval st(type->value.ptr(object), type->value.size, 0);
		auto pt = gettitle(sb, object, type);
		h = field(x, y, width, pt, st, title, 4);
	} else if(type->value.istext()) {
		auto pt = gettitle(sb, object, type);
		h = field(x, y, width, pt, *((const char**)type->value.ptr(object)), title);
	} else {
		auto pt = gettitle(sb, object, type);
		const anyval st(type->value.ptr(object), type->value.size, 0);
		h = field(x, y, width, pt, st, title, *type->value.source, getfntext(object, type), 0, this, getfnallow(object, type));
	/*} else if(type->is(KindScalar)) {
		auto pt = gettitle(temp, zendof(temp), object, type);
		h = group(x, y, width, pt, object, type);*/
	}
	if(!h)
		return 0;
	return h + spacing;
}

void properties::view(const rect& rc) {
	control::view(rc);
	if(!object) {
		auto push_fore = fore;
		fore = colors::text.mix(colors::window, 128);
		text(rc, "У объекта нет свойств", AlignCenterCenter);
		fore = push_fore;
	} else {
		//auto x = rc.x1;
		//auto y = rc.y1;
		//auto width = rc.width();
		//setposition(x, y, width);
		//vertical(x, y, width, object, type);
	}
}