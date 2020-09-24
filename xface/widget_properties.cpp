#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

namespace {
struct rowi {
	void*			data;
	int				index;
};
}

static fntext compare_callback;

static int qsort_compare(const void* v1, const void* v2) {
	auto p1 = (rowi*)v1;
	auto p2 = (rowi*)v2;
	char t1[260]; stringbuilder sb1(t1);
	auto s1 = compare_callback(p1->data, sb1);
	if(!s1)
		s1 = "";
	char t2[260]; stringbuilder sb2(t2);
	auto s2 = compare_callback(p2->data, sb2);
	if(!s2)
		s2 = "";
	return strcmp(s1, s2);
}

static void sort(rowi* storage, unsigned maximum, fntext getname) {
	compare_callback = getname;
	qsort(storage, maximum, sizeof(storage[0]), qsort_compare);
}

static unsigned getrows(const array& source, const void* object, rowi* storage, unsigned maximum, fnallow pallow, fntext getname) {
	auto p = storage;
	auto pe = storage + maximum;
	auto sm = source.getcount();
	for(unsigned i = 0; i < sm; i++) {
		if(pallow && !pallow(object, i))
			continue;
		if(p < pe) {
			p->data = source.ptr(i);
			p->index = i;
			p++;
		}
	}
	auto count = p - storage;
	if(getname)
		sort(storage, count, getname);
	return count;
}

static const char* gettitle(const markup& e) {
	if(!e.title)
		return "";
	if(e.title[0] == '#')
		return zskipsp(e.title + 4);
	return e.title;
}

static int element(int x, int y, int width, const markcontext& ctx, const markup& e);

static int group(int x, int y, int width, const markcontext& ctx, const markup* form) {
	if(!form)
		return 0;
	auto y0 = y;
	for(auto f = form; *f; f++) {
		if(f->ispage() && f != form)
			break;
		y += element(x, y, width, ctx, *f);
	}
	return y - y0;
}

static int checkboxes(int x, int y, int width, const markup& e, void* object) {
	auto ar = e.value.source;
	auto gn = e.list.getname;
	auto pv = e.value.ptr(object);
	rowi storage[512];
	auto y0 = y;
	auto y1 = y0 + 16 * (texth() + 2);
	auto im = getrows(*ar, object, storage, sizeof(storage) / sizeof(storage[0]),
		e.list.allow, e.list.getname);
	if(im > 16)
		width = width / 2;
	for(unsigned i = 0; i < im; i++) {
		auto v = storage[i].data;
		char temp[260]; stringbuilder sb(temp);
		const anyval av((char*)pv + storage[i].index / 8, 1, 1 << (storage[i].index % 8));
		y += checkbox(x, y, width, av, gn(v, sb), 0);
	}
	return y - y0;
}

static int element(int x, int y, int width, const markcontext& ctx, const markup& e) {
	if(e.proc.visible && !e.proc.visible(ctx.object))
		return 0;
	if(e.isgroup()) {
		auto c1 = ctx;
		c1.object = e.value.ptr(ctx.object);
		return group(x, y, width, c1, e.value.type);
	} else if(e.ischeckboxes())
		return checkboxes(x, y, width, e, ctx.object);
	else if(e.value.istext()) {
		char temp[260]; stringbuilder sb(temp);
		auto pt = gettitle(e);
		const anyval av(e.value.ptr(ctx.object), e.value.size, 0);
		const char*& value = *((const char**)e.value.ptr(ctx.object));
		return field(x, y, width, pt, value, ctx.title);
	} else if(e.value.isnum()) {
		char temp[260]; stringbuilder sb(temp);
		auto pt = gettitle(e);
		const anyval av(e.value.ptr(ctx.object), e.value.size, 0);
		auto digits = 8;
		auto width_total = ctx.title + digits*textw('0') - metrics::edit.width() * 2 + 20;
		if(width > width_total)
			width = width_total;
		return field(x, y, width, pt, av, ctx.title, digits);
	} else {
		auto pt = gettitle(e);
		const anyval av(e.value.ptr(ctx.object), e.value.size, 0);
		if(e.value.source)
			return field(x, y, width, pt, av, ctx.title, *e.value.source, e.list.getname, 0, 0, e.list.allow);
	}
	return 0;
}

static void* current_object;

static void setcurpage() {
	auto p = (int*)current_object;
	*p = hot.param;
}

class control_properties : public markcontext, public control {
	const markup*	type;
	int				page, page_maximum;
	const markup* getcurrentpage(const markup** pages, unsigned pages_count) {
		auto ps = pages;
		auto pe = pages + pages_count;
		if(!type->ispage())
			*ps++ = type;
		for(auto p = type; *p; p++) {
			if(!p->ispage())
				continue;
			if(p->proc.visible && !p->proc.visible(object))
				continue;
			if(ps < pe)
				*ps++ = p;
		}
		page_maximum = ps - pages;
		if(page >= page_maximum)
			page = page_maximum - 1;
		if(!page_maximum)
			pages[0] = type;
		return pages[page];
	}
	bool isfocusable() const override {
		return false;
	}
	static const char* getpagename(const void* object, stringbuilder& sb) {
		return gettitle(*((markup*)object));
	}
	const markup* view_pages(int x, int& y, int width) {
		const markup* pages[32];
		auto pm = getcurrentpage(pages, sizeof(pages) / sizeof(pages[0]));
		auto dy = texth() + metrics::padding;
		rect rct = {x, y, x + width, y + dy};
		int current_hilite = -1;
		if(tabs(rct, false, false, (void**)&pages, 0, page_maximum,
			page, &current_hilite, getpagename)) {
			current_object = &page;
			draw::execute(setcurpage, current_hilite);
		}
		line(rct.x1, rct.y2, rct.x2, rct.y2, colors::border);
		//line(rct.x1-1, rct.y2-1, rct.x2-1, rct.y2-1, colors::border.lighten());
		y += dy + metrics::padding;
		return pm;
	}
	void view(const rect& rc) override {
		control::view(rc);
		if(!object) {
			auto push_fore = fore;
			fore = colors::text.mix(colors::window, 128);
			text(rc, "У объекта нет свойств", AlignCenterCenter);
			fore = push_fore;
		} else {
			auto x = rc.x1;
			auto y = rc.y1;
			auto w = rc.width();
			if(show_border)
				setposition(x, y, w);
			auto pm = view_pages(x, y, w);
			group(x, y, w, *this, pm);
		}
	}
public:
	void set(void* object, const markup* type) {
		this->object = object;
		this->type = type;
	}
};

static struct widget_properties : control_properties, control::plugin {
	control& getcontrol() override {
		return *this;
	}
	const char* getlabel(stringbuilder& sb) const override {
		return "Свойства";
	}
	widget_properties() : control::plugin("properties", DockRight) {
		show_background = false;
	}
} widget_control;

void setproperties(void* object, const markup* type) {
	widget_control.set(object, type);
}