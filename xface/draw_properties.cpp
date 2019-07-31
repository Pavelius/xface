#include "crt.h"
#include "draw_properties.h"

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

int properties::element(int x, int y, int width, const bsval& ev) {
	if(!isvisible(ev))
		return 0;
	int h = 0;
	char temp[260];
	if(ev.type->is(KindNumber)) {
		anyval st;
		st.data = ev.type->ptr(ev.data);
		st.size = ev.type->size;
		h = field(x, y, width, gettitle(temp, zendof(temp), ev), st, title, 4);
	} else if(ev.type->is(KindText))
		h = field(x, y, width, gettitle(temp, zendof(temp), ev), *((const char**)ev.type->ptr(ev.data)), title);
	else if(ev.type->is(KindReference)) {
		bsval st;
		st.data = ev.type->ptr(ev.data);
		st.type = ev.type;
		h = combobox(x, y, width, gettitle(temp, zendof(temp), st), st, title, 0);
	}
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