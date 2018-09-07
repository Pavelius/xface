#include "bsreq.h"
#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

const unsigned WidgetMask = 0xF;

struct dlgform : bsval {

	unsigned getflags(const widget& e) const {
		return e.flags;
	}

	bsval getinfo(const char* id) const {
		auto f = type->find(id);
		return {f->type, (void*)f->ptr(data)};
	}

	int vertical(int x, int y, int width, const widget* p) {
		if(!p)
			return 0;
		int y0 = y;
		while(*p) {
			y += element(x, y, width, *p);
			p++;
		}
		return y - y0;
	}

	int horizontal(int x, int y, int width, const widget* p) {
		if(!p)
			return 0;
		int mh = 0;
		int n = 0;
		while(*p) {
			auto w = width * p->width / 12;
			auto x1 = x + width * n / 12;
			auto h = element(x1, y, w, *p);
			if(h > mh)
				mh = h;
			n += p->width;
			p++;
		}
		return mh;
	}

	int group(int x, int y, int width, const widget& e) {
		if(!e.childs)
			return 0;
		int y0 = y;
		if(e.label) {
			setposition(x, y, width); // Первая рамка (может надо двойную ?)
			int x1 = x, y1 = y, w1 = width;
			setposition(x, y, width); // Отступ от рамки
			draw::state push;
			draw::font = metrics::font;
			if(e.label)
				y += texth() + metrics::padding * 2;
			auto w = 0;
			if(e.childs[0].width)
				w = horizontal(x, y, width, e.childs);
			else
				w = vertical(x, y, width, e.childs);
			if(w == 0)
				return 0;
			y += w;
			color c1 = colors::border.mix(colors::window, 128);
			color c2 = c1.darken();
			gradv({x1, y1, x1 + w1, y1 + texth() + metrics::padding * 2}, c1, c2);
			fore = colors::text.mix(c1, 96);
			text(x1 + (w1 - textw(e.label)) / 2, y1 + metrics::padding, e.label);
			rectb({x1, y1, x1 + w1, y}, colors::border);
			y += metrics::padding * 2;
		} else {
			if(e.childs[0].width)
				y += horizontal(x, y, width, e.childs);
			else
				y += vertical(x, y, width, e.childs);
		}
		return y - y0;
	}

	int field(int x, int y, int width, const widget& e) {
		if(!e.id || !e.id[0])
			return 0;
		auto po = getinfo(e.id);
		if(!po)
			return 0;
		char temp[260];
		auto p = po.type->getdata(temp, e.id, po.data, false);
		if(!p)
			return 0;
		auto flags = getflags(e);
		return 0;
	}

	int tabs(int x, int y, int width, const widget& e) {
		if(!e.childs)
			return 0;
		auto po = getinfo(e.id);
		if(!po)
			return 0;
		auto y0 = y;
		const int tab_height = 24 + 4;
		y += 1;
		//sheetline({x, y0, x + width, y + tab_height});
		const widget* data[32];
		auto ps = data;
		auto pe = data + sizeof(data) / sizeof(data[0]);
		for(auto p = e.childs; *p; p++) {
			if(ps < pe)
				*ps++ = p;
		}
		auto count = ps - data;
		auto current = po.get();
		rect rc = {x, y, x + width, y + tab_height};
		int tabs_hilite = -1;
		y += tab_height + metrics::padding;
		return element(x, y, width, e.childs[current]);
	}

	int custom(int x, int y, int width, const widget& e) {
		return 0;
	}

	int decoration(int x, int y, int width, const widget& e) {
		draw::state push;
		auto flags = getflags(e);
		setposition(x, y, width);
		decortext(flags);
		draw::link[0] = 0;
		auto height = draw::textf(x, y, width, e.label) + metrics::padding * 2;
		if(draw::link[0])
			tooltips(link);
		return height;
	}

	int radio(int x, int y, int width, const widget& e) {
		if(!e.label || !e.label[0] || !e.id || !e.id[0])
			return 0;
		auto po = getinfo(e.id);
		if(!po)
			return 0;
		auto flags = getflags(e);
	}

	int check(int x, int y, int width, const widget& e) {
		if(!e.label || !e.label[0] || !e.id || !e.id[0])
			return 0;
		auto po = getinfo(e.id);
		if(!po)
			return 0;
		auto flags = getflags(e);
	}

	int button(int x, int y, int width, const widget& e) {
		auto po = getinfo(e.id);
	}

	int renderno(int x, int y, int width, const widget& e) {
		return 0;
	}

	int element(int x, int y, int width, const widget& e) {
		typedef int (dlgform::*callback)(int, int, int, const widget&);
		static callback methods[] = {
			&dlgform::renderno,
			&dlgform::field, &dlgform::field, &dlgform::check, &dlgform::radio, &dlgform::button, &dlgform::decoration,
			&dlgform::tabs, &dlgform::group
		};
		return (this->*methods[e.flags & WidgetMask])(x, y, width, e);
	}

	dlgform(bsval* variables) {
		data = 0;
	}

};