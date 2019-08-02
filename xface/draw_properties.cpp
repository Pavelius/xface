#include "crt.h"
#include "draw_properties.h"
#include "stringbuilder.h"

using namespace draw;
using namespace draw::controls;

int properties::vertical(int x, int y, int width, const bsval& ev) {
	if(!ev)
		return 0;
	int y0 = y;
	for(auto p = ev.type; *p; p++)
		y += element(x, y, width, bsval(ev.data, p));
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

void properties::treemark(int x, int y, int width, bool isopen) const {
	auto x1 = x + width / 2;
	auto y1 = y + width / 2 - 1;
	rect rc = {x, y, x + width, y + width};
	areas a = area(rc);
	if(a == AreaHilitedPressed) {
		if(hot.key == MouseLeft) {

		}
	}
	color c1 = fore;
	circle(x1, y1, 6, c1);
	line(x1 - 4, y1, x1 + 4, y1, c1);
	if(!isopen)
		line(x1, y1 - 4, x1, y1 + 4, c1);
}

int properties::group(int x, int y, int width, int ident, const char* label, const bsval& ev) const {
	char temp[260];
	draw::state push;
	treemark(x, y + 4, ident, isopen(ev.type));
	addwidth(x, width, ident);
	setposition(x, y, width);
	if(label && label[0])
		titletext(x, y, width, 0, label, title);
	bsval ob;
	ob.data = ev.getptr();
	ob.type = ev.type->type;
	stringbuilder sb(temp);
	for(auto p = ob.type; *p; p++) {
		char tem2[64]; p->get(p->ptr(ob.data), tem2, zendof(tem2));
		sb.addx(", ", tem2, 0);
	}
	setposition(x, y, width);
	text(x, y, temp);
	return ident;
}

int properties::element(int x, int y, int width, const bsval& ev) {
	if(!isvisible(ev))
		return 0;
	int h = 0;
	char temp[260];
	auto ident = texth() + 4 * 2;
	if(ev.type->is(KindNumber)) {
		anyval st;
		addwidth(x, width, ident);
		st.data = ev.type->ptr(ev.data);
		st.size = ev.type->size;
		h = field(x, y, width, gettitle(temp, zendof(temp), ev), st, title, 4);
	} else if(ev.type->is(KindText)) {
		addwidth(x, width, ident);
		h = field(x, y, width, gettitle(temp, zendof(temp), ev), *((const char**)ev.type->ptr(ev.data)), title);
	}
	else if(ev.type->is(KindReference)) {
		addwidth(x, width, ident);
		h = combobox(x, y, width, gettitle(temp, zendof(temp), ev), ev, title, 0);
	} else if(ev.type->is(KindScalar))
		h = group(x, y, width, ident, gettitle(temp, zendof(temp), ev), ev);
	if(!h)
		return 0;
	return h + spacing;
}

void properties::view(const rect& rc) {
	control::view(rc);
	if(!value) {
		auto push_fore = fore;
		fore = colors::text.mix(colors::window, 128);
		text(rc, "У объекта нет свойств", AlignCenterCenter);
		fore = push_fore;
	} else {
		auto x = rc.x1;
		auto y = rc.y1;
		auto width = rc.width();
		setposition(x, y, width);
		vertical(x, y, width, value);
	}
}