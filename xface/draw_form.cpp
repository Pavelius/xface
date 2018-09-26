#include "bsreq.h"
#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

const unsigned WidgetMask = 0xF;

struct dlgform : bsval {

	struct reqcheck : bsval, runable {
		constexpr reqcheck() : bsval(), pw() {}
		constexpr reqcheck(bsval v, const widget& e) : bsval(v), pw(&e) {}
		void			execute() const override { current = *this; draw::execute(callback_proc); }
		int				getid() const { return (int)pw; }
		unsigned		getfocus() const { return is() ? Checked : 0; }
		bool			is() const { return get() == pw->value; }
	private:
		static reqcheck	current;
		static void callback_proc() {
			auto value = current.pw->value;
			if(!value)
				value = 1;
			auto v = current.get() ^ value;
			current.set(v);
		}
		const widget*	pw;
	};

	struct reqradio : bsval, runable {
		constexpr reqradio() : bsval(), pw() {}
		constexpr reqradio(bsval v, const widget& e) : bsval(v), pw(&e) {}
		void			execute() const override { current = *this; draw::execute(callback_proc); }
		int				getid() const { return (int)pw; }
		unsigned		getfocus() const { return is() ? Checked : 0; }
		bool			is() const { return get() == pw->value; }
	private:
		static reqradio	current;
		static void callback_proc() {
			current.set(current.pw->value);
		}
		const widget*	pw;
	};

	unsigned getflags(const widget& e) const {
		return e.flags;
	}

	bsval getinfo(const char* id) const {
		auto f = type->find(id);
		if(!f)
			return {0, 0};
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
			auto x1 = x, y1 = y, w1 = width;
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
		reqradio ev(po, e);
		auto checked = (po.get()==e.value);
		auto flags = getflags(e);
		if(checked)
			flags |= Checked;
		return draw::radio(x, y, width, flags, ev, e.label, e.tips);
	}

	int check(int x, int y, int width, const widget& e) {
		if(!e.label || !e.label[0] || !e.id || !e.id[0])
			return 0;
		auto po = getinfo(e.id);
		if(!po)
			return 0;
		auto value = e.value;
		if(!value)
			value = 1;
		reqcheck ev(po, e);
		auto checked = (po.get()&value)!=0;
		auto flags = getflags(e);
		if(checked)
			flags |= Checked;
		return draw::checkbox(x, y, width, flags, ev, e.label, e.tips);
	}

	int button(int x, int y, int width, const widget& e) {
		if(e.proc)
			return draw::button(x, y, width, getflags(e), cmd(e.proc, e.value), e.label, e.tips);
		else
			return 0;
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

	dlgform(const bsval& value) : bsval(value) {
	}

};

dlgform::reqcheck dlgform::reqcheck::current;
dlgform::reqradio dlgform::reqradio::current;

int draw::render(int x, int y, int width, const bsval& value, const widget* elements) {
	dlgform e(value);
	if(!elements)
		return 0;
	auto y1 = y;
	if(elements[0].width)
		y += e.horizontal(x, y, width, elements);
	else
		y += e.vertical(x, y, width, elements);
	return y - y1;
}