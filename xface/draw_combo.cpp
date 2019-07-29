#include "bsreq.h"
#include "crt.h"
#include "draw.h"
#include "draw_control.h"
#include "screenshoot.h"

using namespace draw;

typedef int(*comparer)(const char* v1, const char* v2);

static unsigned		combo_time;
static char			combo_name[32];
static bsval		combo_value;
static rect			combo_rect;

static int compare_name_as(const char* n1, const char* n2) {
	return strcmp(n1, n2);
}

static int compare_name_ds(const char* n1, const char* n2) {
	return strcmp(n2, n1);
}

static void* get_value(const bsval& e) {
	auto type = e.type;
	if(type->is(KindEnum)) {
		auto b = bsdata::find(type->type);
		if(!b)
			return 0;
		auto index = type->get(type->ptr(e.data));
		return (void*)b->get(index);
	} else
		return (void*)type->get(type->ptr(e.data));
}

static void set_value(const bsval& e1, void* value) {
	auto ps = bsdata::find(e1.type->type);
	if(!ps)
		return;
	auto index = ps->indexof(value);
	if(index == -1)
		return;
	if(e1.type->is(KindEnum))
		e1.set(index);
	else if(e1.type->size == sizeof(void*))
		e1.set((int)value);
}

static const char* get_name(const void* object, const bsreq* field) {
	if(!object || !field)
		return "";
	auto p = *((const char**)field->ptr(object));
	return p ? p : "";
}

static void* find_next(const bsval& e1, comparer c1) {
	auto ps = bsdata::find(e1.type->type);
	if(!ps)
		return 0;
	auto pf = ps->meta->find("name");
	if(!pf)
		return 0;
	auto n1 = get_name(get_value(e1), e1.type->find("name"));
	auto n2 = "";
	const char* e2 = 0;
	auto pe = ps->end();
	for(auto ex = ps->begin(); ex < pe; ex = (char*)ex + ps->size) {
		auto nx = (const char*)pf->get(pf->ptr(ex));
		if(!nx)
			nx = "";
		if(c1(nx, n1) < 0 && (!e2 || c1(nx, n2) > 0)) {
			n2 = nx;
			e2 = ex;
		}
	}
	return (void*)e2;
}

static void* find_name(const bsreq* type, const char* name) {
	auto ps = bsdata::find(type);
	auto pf = ps->meta->find("name");
	if(!pf)
		return 0;
	auto pe = ps->end();
	for(auto ex = ps->begin(); ex < pe; ex = (char*)ex + ps->size) {
		auto nx = (const char*)pf->get(pf->ptr(ex));
		if(!nx || nx[0] == 0)
			continue;
		if(matchuc(nx, name))
			return (void*)ex;
	}
	return 0;
}

static void combo_previous() {
	auto pn = find_next(combo_value, compare_name_as);
	if(pn)
		set_value(combo_value, pn);
}

static void combo_next() {
	auto pn = find_next(combo_value, compare_name_ds);
	if(pn)
		set_value(combo_value, pn);
}

static void combo_find_name() {
	auto pn = find_name(combo_value.type->type, combo_name);
	if(pn)
		set_value(combo_value, pn);
}

static int compare_by_order(void* v1, void* v2, const bsreq* field) {
	auto n1 = get_name(*((void**)v1), field);
	auto n2 = get_name(*((void**)v2), field);
	return strcmp(n1, n2);
}

struct combo_list : controls::list, adat<void*, 64> {

	const bsreq*	field;
	const bsdata&	metadata;

	combo_list(const bsdata& metadata) : metadata(metadata) {
		field = metadata.meta->find("name");
		if(!field)
			field = metadata.meta->find("id");
	}

	const char* getname(char* result, const char* result_max, int line, int column) const override {
		auto p = (const char*)field->get(field->ptr(data[line]));
		return p ? p : "";
	}

	int	getmaximum() const {
		return getcount();
	}

	void mouseselect(int id, bool pressed) override {
		if(pressed)
			list::mouseselect(id, pressed);
		else
			draw::execute(buttonok);
	}

	bool keyinput(unsigned id) override {
		switch(id) {
		case KeyEnter:
			breakmodal(1);
			hot.zero();
			break;
		default: return list::keyinput(id);
		}
		return true;
	}

	int find(const void* object) const {
		for(auto p : *this) {
			if(p == object)
				return indexof(p);
		}
		return -1;
	}

	void sort() {
		for(auto step = count / 2; step > 0; step /= 2) {
			for(auto i = step; i < count; i++) {
				for(int j = i - step; j >= 0 && compare_by_order(data + j, data + (j + step), field); j -= step) {
					auto temp = data[j];
					data[j] = data[j + step];
					data[j + step] = temp;
				}
			}
		}
	}

};

static void show_drop_down() {
	auto ps = bsdata::find(combo_value.type->type);
	if(!ps)
		return;
	combo_list list(*ps);
	list.hilite_odd_lines = false;
	if(!list.field)
		return;
	auto pe = ps->end();
	auto value = get_value(combo_value);
	for(auto p = ps->begin(); p < pe; p += ps->size)
		list.add((void*)p);
	list.sort();
	list.pixels_per_line = list.getrowheight();
	list.lines_per_page = imin(list.getcount(), 7);
	rect rc = combo_rect;
	rc.y1 = rc.y2;
	rc.y2 = rc.y1 + list.lines_per_page*list.pixels_per_line + 1;
	if(rc.y2 > getheight() - 2) {
		rc.y2 = getheight() - 2;
		list.lines_per_page = list.getlinesperpage(rc.height());
	}
	if(rc.y1 > rc.y2 - (list.pixels_per_line + 1)) {
		rc.y1 = rc.y2 - (list.pixels_per_line + 1);
		list.lines_per_page = list.getlinesperpage(rc.height());
	}
	list.current = list.find(value);
	list.ensurevisible();
	if(dropdown(rc, list)) {
		auto value = list.data[list.current];
		set_value(combo_value, value);
	}
}

void draw::combobox(const rect& rc, const bsval& cmd) {
	combo_value = cmd;
	combo_rect = rc;
	show_drop_down();
}

int	draw::combobox(int x, int y, int width, unsigned flags, const bsval& cmd, const char* header_label, const char* tips, int header_width) {
	draw::state push;
	setposition(x, y, width);
	decortext(flags);
	if(header_label && header_label[0])
		titletext(x, y, width, flags, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	focusing((int)cmd.type->ptr(cmd.data), flags, rc);
	auto focused = isfocused(flags);
	auto result = false;
	auto a = area(rc);
	if(isdisabled(flags)) {
		gradv(rc, colors::button.lighten(), colors::button.darken());
		rectb(rc, colors::border.mix(colors::window));
	} else {
		if((a == AreaHilited || a == AreaHilitedPressed) && hot.key == MouseLeft && !hot.pressed)
			result = true;
		color active = colors::button.mix(colors::edit, 128);
		switch(a) {
		case AreaHilited: gradv(rc, active.lighten(), active.darken()); break;
		case AreaHilitedPressed: gradv(rc, active.darken(), active.lighten()); break;
		default: gradv(rc, colors::button.lighten(), colors::button.darken()); break;
		}
		rectb(rc, colors::border);
	}
	rect rco = rc;
	if(a == AreaHilited || a == AreaHilitedPressed) {
		switch(hot.key) {
		case MouseWheelDown:
			combo_value = cmd;
			execute(combo_next);
			break;
		case MouseWheelUp:
			combo_value = cmd;
			execute(combo_previous);
			break;
		case MouseLeft:
			if(!hot.pressed) {
				combo_value = cmd;
				combo_rect = rc;
				execute(show_drop_down);
			}
			break;
		}
	}
	if(focused) {
		switch(hot.key) {
		case InputSymbol:
			if(hot.param >= 0x20) {
				unsigned time = clock();
				if(!combo_time || (time - combo_time) > 1000)
					combo_name[0] = 0;
				combo_time = time;
				if(zlen(combo_name) < sizeof(combo_name) - 1) {
					zcat(combo_name, (char)hot.param);
					combo_value = cmd;
					execute(combo_find_name);
				}
			}
			break;
		case KeyUp:
			combo_value = cmd;
			execute(combo_previous);
			break;
		case KeyDown:
			combo_value = cmd;
			execute(combo_next);
			break;
		case KeyEnter:
			combo_value = cmd;
			combo_rect = rc;
			execute(show_drop_down);
			break;
		}
	}
	rco.offset(2, 2);
	if(focused)
		rectx(rco, colors::black);
	rco.offset(2, 2);
	auto name = get_name(get_value(cmd), cmd.type);
	if(name && name[0])
		textc(rco.x1, rco.y1, rco.width(), name);
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc.height() + metrics::padding * 2;
}