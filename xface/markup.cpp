#include "draw_control.h"
#include "markup2.h"

using namespace draw;

namespace {

struct rowi {
	void*				data;
	int					index;
};
}

static const char* gettitle(const markup* pm) {
	if(!pm || !pm->title || pm->title[0] != '#')
		return "";
	return zskipsp(pm->title + 4);
}

static void next_page() {
	//((edit_control*)hot.param)->page++;
}

static void prev_page() {
	//((edit_control*)hot.param)->page--;
}

static int group(int x, int y, int width, const contexti& ctx, const markup* form) {
	if(!form)
		return 0;
	auto y0 = y;
	for(auto f = form; *f; f++) {
		if(f->ispage())
			break;
		auto h = element(x, y, width, ctx, *f);
		if(!h)
			continue;
		y += h + 2;
	}
	return y - y0;
}

static int element(int x, int y, int width, const contexti& ctx, const markup& e) {
	if(e.proc.visible && !e.proc.visible(ctx.object))
		return 0;
	if(e.isgroup()) {
		contexti c1 = ctx;
		c1.object = e.value.ptr(ctx.object);
		return group(x, y, width, c1, e.value.type);
		/*} else if(e.ischeckboxes())
			return checkboxes(x, y, width, e, ctx.object, e.value.size);
		else if(e.value.mask)
			return checkbox(x, y, e.title, e, ctx.object, e.value.mask);*/
	}/* else
		return field(x, y, width, e.title, ctx.object, ctx.title, e);*/
}

static const markup* getcurrentpage(int& page, int& page_maximum, const markup** pages, unsigned page_count, const markup* elements) {
	auto ps = pages;
	auto pe = pages + sizeof(pages) / sizeof(pages[0]);
	if(!elements->ispage())
		*ps++ = elements;
	for(auto p = elements; *p; p++) {
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
		pages[0] = elements;
	return pages[page];
}