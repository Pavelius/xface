#include "crt.h"
#include "draw_control.h"
#include "screenshoot.h"
#include "valuelist.h"

using namespace draw;
using namespace draw::controls;

namespace {
struct autocomplete : draw::controls::list {
	arem<listelement*>	source;
	bool				show_images;
	const sprite*		images;
	constexpr autocomplete() : source(), show_images(false), images() {}
	const char*	getname(char* result, const char* result_max, int line, int column) const override {
		switch(column) {
		case 0: return source.data[line]->text;
		}
		return "";
	}
	int	getmaximum() const {
		return source.count;
	}
	int getimage(int line) const {
		return source.data[line]->image;
	}
	listelement* getcurrent() const {
		if(current < getmaximum())
			return source.data[current];
		return 0;
	}
	void row(const rect& rc, int index) override {
		char temp[260]; temp[0] = 0;
		rowhilite(rc, index);
		auto rt = rc;
		if(show_images) {
			image(rt.x1 + rt.height() / 2, rt.y1 + rt.height() / 2, images, getimage(index), 0);
			rt.x1 += rt.height() - 4;
		}
		auto p = getname(temp, temp + sizeof(temp) / sizeof(temp[0]) - 1, index, 0);
		if(p)
			draw::textc(rt.x1 + 4, rt.y1 + 4, rt.width() - 4 * 2, p);
	}
	bool szmatch(const char* name, const char* text) {
		while(true) {
			auto s1 = szget(&name);
			if(!s1)
				return true;
			auto s2 = szget(&text);
			if(szupper(s1) != szupper(s2))
				return false;
		}
	}
	static int compare(const void* v1, const void* v2) {
		auto e1 = *((listelement**)v1);
		auto e2 = *((listelement**)v2);
		return strcmp(e1->text, e2->text);
	}
	void update(const char* filter, valuelist& source_list, rect& rc) {
		rc.y2 = rc.y1;
		source.clear();
		for(auto& e : source_list) {
			if(filter && filter[0] && !szmatch(filter, e.text))
				continue;
			source.add(&e);
		}
		if(!source)
			return;
		qsort(source.data, source.getcount(), sizeof(source.data[0]), compare);
		pixels_per_line = getrowheight();
		lines_per_page = imin(source.getcount(), 7);
		rc.y2 = rc.y1 + lines_per_page*pixels_per_line + 1;
		if(rc.y2 > getheight() - 2) {
			rc.y2 = getheight() - 2;
			lines_per_page = getlinesperpage(rc.height());
		}
		current = 0;
		ensurevisible();
	}
};
struct fillable {
	virtual int			getcurrent() const = 0;
	virtual bool		isready() const = 0;
	virtual void		update(const char* filter, rect& rc) = 0;
};
}

listelement* choose_text(int x, int y, int width, control& e, fillable& vs, const char* start_filter) {
	char filter[32] = {};
	if(start_filter)
		zcpy(filter, start_filter, sizeof(filter) - 1);
	rect rc = {x, y, x + width, y + 100};
	screenshoot screen;
	auto push_focus = getfocus();
	setfocus((int)&e, true);
	vs.update(filter, rc);
	while(ismodal()) {
		screen.restore();
		if(rc.y1<rc.y2)
			e.view(rc);
		domodal();
		switch(hot.key) {
		case KeyEscape:
			breakmodal(0);
			hot.zero();
			break;
		case KeyTab:
		case KeyTab | Shift:
			breakmodal(0);
			break;
		case MouseLeft:
		case MouseLeft + Ctrl:
		case MouseLeft + Shift:
		case MouseLeftDBL:
		case MouseLeftDBL + Ctrl:
		case MouseLeftDBL + Shift:
			if(!areb(rc))
				breakmodal(0);
			else {
				if(!hot.pressed) {
					breakmodal(vs.getcurrent());
					hot.zero();
				}
			}
			break;
		case InputSymbol:
			if(hot.param > 32) {
				szput(zend(filter), hot.param);
				vs.update(filter, rc);
			}
			break;
		case KeyBackspace:
			if(filter[0])
				filter[zlen(filter) - 1] = 0;
			vs.update(filter, rc);
			break;
		case KeyEnter:
			breakmodal(vs.getcurrent());
			hot.zero();
			break;
		}
	}
	setfocus(push_focus, true);
	return (listelement*)getresult();
}

listelement* choose(int x, int y, int width, valuelist& vs, const char* start_filter, const struct sprite* images) {
	char filter[32] = {};
	if(start_filter)
		zcpy(filter, start_filter, sizeof(filter)-1);
	rect rc = {x, y, x + width, y + 100};
	autocomplete e;
	screenshoot screen;
	auto push_focus = getfocus();
	setfocus((int)&e, true);
	e.images = images;
	if(e.images)
		e.show_images = true;
	e.update(filter, vs, rc);
	while(ismodal()) {
		if(!e.getmaximum()) {
			breakmodal(0);
			continue;
		}
		screen.restore();
		if(rc.y1<rc.y2)
			e.view(rc);
		domodal();
		switch(hot.key) {
		case KeyEscape:
			breakmodal(0);
			hot.zero();
			break;
		case KeyTab:
		case KeyTab | Shift:
			breakmodal(0);
			break;
		case MouseLeft:
		case MouseLeft + Ctrl:
		case MouseLeft + Shift:
		case MouseLeftDBL:
		case MouseLeftDBL + Ctrl:
		case MouseLeftDBL + Shift:
			if(!areb(rc))
				breakmodal(0);
			else {
				if(!hot.pressed) {
					breakmodal((int)e.getcurrent());
					hot.zero();
				}
			}
			break;
		case InputSymbol:
			if(hot.param > 32) {
				szput(zend(filter), hot.param);
				e.update(filter, vs, rc);
			}
			break;
		case KeyBackspace:
			if(filter[0])
				filter[zlen(filter) - 1] = 0;
			e.update(filter, vs, rc);
			break;
		case KeyEnter:
			breakmodal((int)e.getcurrent());
			hot.zero();
			break;
		}
	}
	setfocus(push_focus, true);
	return (listelement*)getresult();
}