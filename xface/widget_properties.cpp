#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

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
		if(f->ispage())
			break;
		y += element(x, y, width, ctx, *f);
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
	} else if(e.ischeckboxes()) {
	} else if(e.value.istext()) {
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
		auto width_total = ctx.title + digits*textw('0') - metrics::edit.width()*2 + 20;
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

class control_properties : public markcontext, public control {
	const markup*	type;
	int				page, page_maximum;
	const markup* getcurrentpage() {
		const markup* pages[32];
		auto ps = pages;
		auto pe = pages + sizeof(pages) / sizeof(pages[0]);
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
			auto pm = getcurrentpage();
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