#include "bsdata.h"
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
	if(type->isenum) {
		auto b = bsdata::find(type->type);
		if(!b)
			return 0;
		auto index = type->get(type->ptr(e.data));
		return b->get(index);
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
	if(e1.type->isenum)
		e1.set(index);
	else if(e1.type->size == sizeof(void*))
		e1.set((int)value);
}

static const char* get_name(const bsreq* type, const void* object) {
	if(!object || !type)
		return "";
	auto pf = type->type->find("name");
	if(pf) {
		auto name = (const char*)pf->get(pf->ptr(object));
		if(name)
			return name;
	}
	return "";
}

static int compare_objects(const void* p1, const void* p2) {
	auto n1 = get_name(combo_value.type, *((void**)p1));
	auto n2 = get_name(combo_value.type, *((void**)p2));
	return strcmp(n1, n2);
}

static void* find_next(const bsval& e1, comparer c1) {
	auto ps = bsdata::find(e1.type->type);
	if(!ps)
		return 0;
	auto pf = ps->fields->find("name");
	if(!pf)
		return 0;
	auto n1 = get_name(e1.type, get_value(e1));
	auto n2 = "";
	void* e2 = 0;
	auto pe = ps->end();
	for(void* ex = ps->begin(); ex < pe; ex = (char*)ex + ps->size) {
		auto nx = (const char*)pf->get(pf->ptr(ex));
		if(!nx)
			nx = "";
		if(c1(nx, n1) < 0 && (!e2 || c1(nx, n2) > 0)) {
			n2 = nx;
			e2 = ex;
		}
	}
	return e2;
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
}

struct combo_list : controls::list, adat<void*, 64> {
	const bsreq*		field;
	const bsdata&		metadata;

	combo_list(const bsdata& metadata) : metadata(metadata), field(metadata.fields->find("name")) {}

	const char* getname(char* result, const char* result_max, int line, int column) const override {
		auto p = (const char*)field->get(field->ptr(data[line]));
		return p ? p : "";
	}

	int	getmaximum() const {
		return getcount();
	}

	void mouseselect(int id, bool pressed, int index) override {
		select(index);
		keyenter(0);
	}

	void keyenter(int id) override {
		breakmodal(1);
	}

};

static void show_drop_down() {
	auto ps = bsdata::find(combo_value.type->type);
	if(!ps)
		return;
	combo_list list(*ps);
	if(!list.field)
		return;
	auto pe = ps->end();
	auto value = get_value(combo_value);
	for(auto p = ps->begin(); p < pe; p += ps->size) {
		if(value == p)
			list.current = list.count;
		list.add(p);
	}
	qsort(list.data, list.getcount(), sizeof(list.data[0]), compare_objects);
	rect rc = combo_rect;
	rc.y1 = rc.y2;
	rc.y2 = rc.y1 + imin(list.getcount(), 12) *list.getrowheight() + 1;
	if(dropdown(rc, list)) {
		auto value = list.data[list.current];
		set_value(combo_value, value);
	}
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
			if(hot.pressed) {
				combo_value = cmd;
				combo_rect = rc;
				execute(show_drop_down);
			}
			break;
		}
	}
	if(focused) {
		unsigned time;
		switch(hot.key) {
		case InputSymbol:
			time = clock();
			if(!combo_time || (time - combo_time) > 1000)
				combo_name[0] = 0;
			combo_time = time;
			if(zlen(combo_name) < sizeof(combo_name) - 1) {
				zcat(combo_name, (char)hot.param);
				combo_value = cmd;
				execute(combo_find_name);
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
		case F4:
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
	auto name = get_name(cmd.type, get_value(cmd));
	if(name[0])
		textc(rco.x1, rco.y1, rco.width(), name);
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc.height() + metrics::padding * 2;
}