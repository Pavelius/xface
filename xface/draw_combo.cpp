#include "bsreq.h"
#include "crt.h"
#include "draw.h"
#include "draw_control.h"
#include "screenshoot.h"

using namespace draw;

static unsigned		combo_time;
static char			combo_name[32];
static bsval		combo_value;
static rect			combo_rect;
static listproc*	combo_choose;

static const char* getv(const void* object, const bsreq* type) {
	if(!object || !type)
		return "";
	auto p = (const char*)type->get(type->ptr(object));
	if(!p)
		return "";
	return p;
}

static void* getv(const bsval& e) {
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

static void setv(const bsval& e, void* value) {
	auto type = e.type;
	if(type->is(KindEnum)) {
		auto ps = bsdata::find(type->type);
		if(!ps)
			return;
		auto index = ps->indexof(value);
		if(index == -1)
			return;
		e.set(index);
	} else
		e.set((int)value);
}

static void* find_name(const bsreq* type, const char* name) {
	auto ps = bsdata::find(type);
	if(!ps)
		return 0;
	auto pf = type->getname();
	if(!pf)
		return 0;
	auto pe = ps->end();
	for(auto ex = ps->begin(); ex < pe; ex = (char*)ex + ps->size) {
		auto nx = getv(ex, pf);
		if(nx[0] == 0)
			continue;
		if(matchuc(nx, name))
			return (void*)ex;
	}
	return 0;
}

static void combo_find_name() {
	auto pn = find_name(combo_value.type->type, combo_name);
	if(pn)
		setv(combo_value, pn);
}

struct combo_list : controls::list, adat<void*, 64> {

	const bsreq* field;

	combo_list() : field(0) {
	}

	const char* getname(char* result, const char* result_max, int line, int column) const override {
		return field->gets(field->ptr(data[line]));
	}

	int compare_by_order(void* v1, void* v2) {
		auto p1 = *((void**)v1);
		auto p2 = *((void**)v2);
		auto n1 = "";
		if(p1)
			n1 = field->gets(field->ptr(p1));
		auto n2 = field->gets(p2);
		if(p2)
			n2 = field->gets(field->ptr(p2));
		return strcmp(n1, n2);
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
				for(int j = i - step; j >= 0 && compare_by_order(data + j, data + (j + step)); j -= step) {
					auto temp = data[j];
					data[j] = data[j + step];
					data[j + step] = temp;
				}
			}
		}
	}

};

static void standart_combo_choose(adat<void*, 64>& result, const bsreq** name_requisit, void* source) {
	auto ps = bsdata::find(combo_value.type->type);
	if(ps) {
		if(name_requisit)
			*name_requisit = ps->meta->getname();
		auto pe = ps->end();
		for(auto p = ps->begin(); p < pe; p += ps->size)
			result.add((void*)p);
	} else if(combo_value.type->type == bsmeta<bsreq>::meta) {
		if(name_requisit)
			*name_requisit = bsmeta<bsreq>::meta->find("id");
		for(auto ps = bsdata::first; ps; ps = ps->next)
			result.add((void*)ps->meta);
	}
}

static void show_drop_down() {
	combo_list list;
	if(!combo_choose)
		combo_choose = standart_combo_choose;
	combo_choose(list, &list.field, (void*)combo_value.type);
	if(list.getcount() == 0)
		return;
	list.hilite_odd_lines = false;
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
	auto value = getv(combo_value);
	list.current = list.find(value);
	list.ensurevisible();
	if(dropdown(rc, list)) {
		auto value = list.data[list.current];
		setv(combo_value, value);
	}
}

void draw::combobox(const rect& rc, const bsval& cmd, listproc choose, bool instant) {
	combo_value = cmd;
	combo_rect = rc;
	combo_choose = choose;
	if(instant)
		show_drop_down();
	else
		execute(show_drop_down);
}

int	draw::combobox(int x, int y, int width, const char* header_label, const bsval& cmd, int header_width, const char* tips, listproc choose) {
	draw::state push;
	setposition(x, y, width);
	decortext(0);
	if(header_label && header_label[0])
		titletext(x, y, width, 0, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	if(rc.width() <= 0)
		return rc.height() + metrics::padding * 2;
	auto focused = focusing((int)cmd.getptr(), rc);
	auto result = false;
	auto a = area(rc);
	if((a == AreaHilited || a == AreaHilitedPressed) && hot.key == MouseLeft && !hot.pressed)
		result = true;
	color active = colors::button.mix(colors::edit, 128);
	switch(a) {
	case AreaHilited: gradv(rc, active.lighten(), active.darken()); break;
	case AreaHilitedPressed: gradv(rc, active.darken(), active.lighten()); break;
	default: gradv(rc, colors::button.lighten(), colors::button.darken()); break;
	}
	rectb(rc, colors::border);
	rect rco = rc;
	auto execute_drop_down = false;
	if(a == AreaHilited || a == AreaHilitedPressed) {
		switch(hot.key) {
		case MouseLeft:
			if(!hot.pressed)
				execute_drop_down = true;
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
		case KeyEnter:
			execute_drop_down = true;
			break;
		}
	}
	if(execute_drop_down)
		combobox(rc, cmd, choose, false);
	rco.offset(2, 2);
	if(focused)
		rectx(rco, colors::black);
	rco.offset(2, 2);
	textc(rco.x1, rco.y1, rco.width(), cmd.type->gets(cmd.type->ptr(cmd.data)));
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc.height() + metrics::padding * 2;
}