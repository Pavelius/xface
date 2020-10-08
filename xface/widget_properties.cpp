#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

namespace {
struct rowi {
	void*			data;
	int				index;
};
struct resulti {
	void*			data;
	unsigned char	size;
	unsigned		param;
	const markup*	type;
	unsigned		count;
	bool checkcount() {
		if(count == 0xFFFFFFFF)
			return false;
		else if(count > 0) {
			count--;
			return false;
		}
		count = 0xFFFFFFFF;
		return true;
	}
	void field(const markup& e, void* data, unsigned size, unsigned param) {
		if(checkcount()) {
			this->data = data;
			this->size = size;
			this->param = param;
			this->type = &e;
		}
	}
	bool field(const markup& e, void* object) {
		if(!this)
			return false;
		field(e, e.value.ptr(object), e.value.size, e.value.mask);
		return true;
	}
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
		return skipsp(e.title + 4);
	return e.title;
}

static int element(int x, int y, int width, const markcontext& ctx, const markup& e, resulti* result_markup);

static int group(int x, int y, int width, const markcontext& ctx, const markup* form, resulti* result_markup) {
	if(!form)
		return 0;
	auto y0 = y;
	for(auto f = form; *f; f++) {
		if(f->ispage() && f != form)
			break;
		y += element(x, y, width, ctx, *f, result_markup);
	}
	return y - y0;
}

static int checkboxes(int x, int y, int width, const markup& e, void* object, resulti* result_markup) {
	auto ar = e.value.source;
	auto pv = e.value.ptr(object);
	rowi storage[512];
	auto im = getrows(*ar, object, storage, sizeof(storage) / sizeof(storage[0]),
		e.list.allow, e.list.getname);
	if(im > 16)
		width = width / 2;
	if(result_markup) {
		result_markup->field(e, (char*)pv + storage[0].index / 8, 1, 1 << (storage[0].index % 8));
		return 0;
	}
	auto y0 = y;
	//auto y1 = y0 + 16 * (texth() + 2);
	auto gn = e.list.getname;
	for(unsigned i = 0; i < im; i++) {
		auto v = storage[i].data;
		char temp[260]; stringbuilder sb(temp);
		const anyval av((char*)pv + storage[i].index / 8, 1, 1 << (storage[i].index % 8));
		y += checkbox(x, y, width, av, gn(v, sb), 0);
	}
	return y - y0;
}

static int element(int x, int y, int width, const markcontext& ctx, const markup& e, resulti* result_markup) {
	if(e.proc.visible && !e.proc.visible(ctx.object))
		return 0;
	if(e.isgroup()) {
		auto c1 = ctx;
		c1.object = e.value.ptr(ctx.object);
		return group(x, y, width, c1, e.value.type, result_markup);
	} else if(e.ischeckboxes())
		return checkboxes(x, y, width, e, ctx.object, result_markup);
	else if(e.ischeckbox()) {
		if(result_markup->field(e, ctx.object))
			return 0;
		const anyval av(e.value.ptr(ctx.object), e.value.size, e.value.mask);
		return checkbox(x, y, width, av, gettitle(e), 0);
	} else if(e.value.istext()) {
		if(result_markup->field(e, ctx.object))
			return 0;
		char temp[260]; stringbuilder sb(temp);
		auto pt = gettitle(e);
		const anyval av(e.value.ptr(ctx.object), e.value.size, 0);
		const char*& value = *((const char**)e.value.ptr(ctx.object));
		return field(x, y, width, pt, value, ctx.title);
	} else if(e.value.isnum()) {
		if(result_markup->field(e, ctx.object))
			return 0;
		const anyval av(e.value.ptr(ctx.object), e.value.size, 0);
		char temp[260]; stringbuilder sb(temp);
		auto pt = gettitle(e);
		auto digits = 8;
		auto width_total = ctx.title + digits*textw('0') - metrics::edit.width() * 2 + 20;
		if(width > width_total)
			width = width_total;
		return field(x, y, width, pt, av, ctx.title, digits);
	} else {
		if(result_markup->field(e, ctx.object))
			return 0;
		const anyval av(e.value.ptr(ctx.object), e.value.size, 0);
		auto pt = gettitle(e);
		if(e.value.source)
			return field(x, y, width, pt, av, ctx.title, *e.value.source, ctx.object, e.list);
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
		if(page_maximum > 1) {
			auto dy = texth() + metrics::padding;
			rect rct = {x, y, x + width, y + dy};
			int current_hilite = -1;
			if(tabs(rct, false, false, (void**)&pages, 0, page_maximum,
				page, &current_hilite, getpagename)) {
				current_object = &page;
				draw::execute(setcurpage, current_hilite);
			}
			line(rct.x1, rct.y2, rct.x2, rct.y2, colors::border);
			y += dy + metrics::padding;
		}
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
			group(x, y, w, *this, pm, 0);
		}
	}
public:
	void get(resulti& result) {
		const markup* pages[32];
		auto pm = getcurrentpage(pages, sizeof(pages) / sizeof(pages[0]));
		group(0, 0, 320, *this, pm, &result);
	}
	void set(void* object, const markup* type) {
		this->object = object;
		this->type = type;
	}
	const markup* getmarkup() const {
		return type;
	}
};

static struct widget_properties : control_properties, control::plugin {
	control* getcontrol() override {
		return this;
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

void* getpropertiesobject() {
	return widget_control.object;
}

const markup* getpropertiesmarkup() {
	return widget_control.getmarkup();
}

void setpropertiesfocus() {
	resulti result = {};
	widget_control.get(result);
	if(result.data) {
		const anyval av(result.data, result.size, result.param);
		setfocus(av, true);
	}
}