#include "crt.h"
#include "draw_properties.h"
#include "stringbuilder.h"

using namespace draw;
using namespace draw::controls;

int properties::translate::compare(const void* v1, const void* v2) {
	return strcmp(((translate*)v1)->id, ((translate*)v2)->id);
}

int properties::vertical(int x, int y, int width, void* object, const bsreq* type) {
	if(!object)
		return 0;
	int y0 = y;
	for(auto p = type; *p; p++)
		y += element(x, y, width, object, p);
	return y - y0;
}

void properties::open(void* object) {
	if(isopen(object))
		return;
	opened.add(object);
}

void properties::close(void* object) {
	auto i = opened.indexof(object);
	if(i != -1)
		opened.remove(i);
}

bool properties::cmdopen(bool run) {
	auto object = (void*)hot.param;
	if(run) {
		if(isopen(object))
			close(object);
		else
			open(object);
	}
	return true;
}

static void* current_object;

void properties::toggle(bool run) {

}

void properties::treemark(int x, int y, int width, void* object, bool isopen) const {
	auto x1 = x + width / 2;
	auto y1 = y + width / 2 - 1;
	rect rc = {x, y, x + width, y + width};
	auto a = ishilite(rc);
	if(a && hot.pressed && hot.key == MouseLeft) {
		current_object = object;
		execute((fncmd)&properties::toggle);
	}
	color c1 = fore;
	circle(x1, y1, 6, c1);
	line(x1 - 4, y1, x1 + 4, y1, c1);
	if(!isopen)
		line(x1, y1 - 4, x1, y1 + 4, c1);
}

static const bsreq* current_type;

int properties::group(int x, int y, int width, const char* label, void* object, const bsreq* type) {
	draw::state push;
	auto y1 = y;
	auto dy = texth() + 4;
	auto opened = isopen(type);
	rect rc = {x, y, x + width, y + dy};
	gradv(rc, colors::form, colors::border);
	auto focused = draw::isfocused(rc, anyval(object, 0, 0));
	if(label && label[0])
		text(x + 4, y + 2, label);
	auto need_open = false;
	if(focused)
		rectx(rc, colors::border);
	switch(hot.key) {
	case MouseLeft:
		if(ishilite(rc) && !hot.pressed)
			need_open = true;
		break;
	case KeySpace:
	case KeyRight:
		if(focused)
			need_open = true;
		break;
	}
	y += dy;
	if(opened)
		y += vertical(x, y, width, type->ptr(object), type->type);
	if(need_open) {
		current_type = type;
		execute((fncmd)&properties::toggle);
	}
	return y - y1;
}

const char* properties::gettitle(char* result, const char* result_maximum, const void* object, const bsreq* type) const {
	auto pv = type->id;
	translate kv = {pv, 0};
	auto fv = (translate*)bsearch(&kv, dictionary.data, dictionary.count, sizeof(dictionary.data[0]), translate::compare);
	if(fv)
		return fv->value;
	return pv;
}

void properties::focusfirst() {
	const anyval st(type->ptr(object), type->size, 0);
	draw::setfocus(st, true);
}

int properties::element(int x, int y, int width, void* object, const bsreq* type) {
	if(!isvisible(object, type))
		return 0;
	int h = 0;
	char temp[260];
	if(type->is(KindNumber)) {
		const anyval st(type->ptr(object), type->size, 0);
		auto pt = gettitle(temp, zendof(temp), object, type);
		h = field(x, y, width, pt, st, title, 4);
	} else if(type->is(KindText)) {
		auto pt = gettitle(temp, zendof(temp), object, type);
		h = field(x, y, width, pt, *((const char**)type->ptr(object)), title);
	} else if(type->is(KindReference) && type->source) {
		auto pt = gettitle(temp, zendof(temp), object, type);
		const anyval st(type->ptr(object), type->size, 0);
		h = field(x, y, width, pt, st, title, *type->source, table::getenumid);
	} else if(type->is(KindScalar)) {
		auto pt = gettitle(temp, zendof(temp), object, type);
		h = group(x, y, width, pt, object, type);
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
		auto x = rc.x1;
		auto y = rc.y1;
		auto width = rc.width();
		setposition(x, y, width);
		vertical(x, y, width, object, type);
	}
}