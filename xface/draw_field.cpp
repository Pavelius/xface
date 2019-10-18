#include "anyval.h"
#include "crt.h"
#include "draw.h"
#include "draw_control.h"

using namespace draw;

static eventproc	choose_proc;
static rect			cmb_rect;
static acol			cmb_source;
static anyval		cmb_var;
static pgetname		cmb_getname;

static const char* getvalue(const anyval& v, bstype_s t, char* result, const char* result_end) {
	if(!v)
		return "";
	const char* p;
	switch(t) {
	case KindNumber:
		return szprint(result, result_end, "%1i", v.get());
	case KindText:
		p = (const char*)v.get();
		if(p)
			return szprint(result, result_end, p);
		result[0] = 0;
		return result;
	default: return "";
	}
}

static void setvalue(const anyval& v, bstype_s t, const char* result) {
	if(!v)
		return;
	int value = 0;
	switch(t) {
	case KindNumber: value = sz2num(result); break;
	case KindText: value = (int)szdup(result); break;
	}
	v.set(value);
}

static class edit_driver : public controls::textedit {
	char			source[2048];
	anyval			value;
	bstype_s		type;
public:
	constexpr edit_driver() : textedit(source, sizeof(source) / sizeof(source), false),
		value(), source(), type(KindNumber) {
	}
	bool isfocusable() const override {
		return false;
	}
	bool isfocused() const override {
		return draw::isfocused(value);
	}
	void load() {
		auto p = getvalue(value, type, source, source + sizeof(source) / sizeof(source[0]) - 1);
		if(p != source)
			zcpy(source, p, sizeof(source));
		invalidate();
	}
	void save() {
		setvalue(value, type, source);
	}
	void update(const anyval& ev, bstype_s et, int digits = -1) {
		if(value == ev)
			return;
		value = ev;
		type = et;
		if(digits == -1)
			setcount(sizeof(source) / sizeof(source[0]) - 1);
		else
			setcount(digits);
		load();
		select_all(true);
	}
	void clear() {
		textedit::clear();
		value.clear();
	}
} cedit;

void draw::savefocus() {
	if(cedit.isfocused()) {
		cedit.save();
		cedit.clear();
	}
}

static void execute_choose() {
	if(!choose_proc)
		return;
	choose_proc();
	cedit.load();
	cedit.select_all(true);
	cedit.invalidate();
}

static void field_up() {
	cmb_var.set(cmb_var.get() + 1);
	cedit.load();
	cedit.invalidate();
}

static void field_down() {
	cmb_var.set(cmb_var.get() - 1);
	cedit.load();
	cedit.invalidate();
}

void draw::field(const rect& rco, unsigned flags, const anyval& ev, int digits, bool increment, bstype_s type, eventproc pchoose) {
	if(rco.width() <= 0)
		return;
	rect rc = rco;
	const unsigned edit_mask = AlignMask | TextSingleLine | TextBold | TextStroke;
	draw::rectf(rc, colors::window);
	draw::rectb(rc, colors::border);
	auto focused = isfocused(flags);
	if(increment) {
		auto result = addbutton(rc, focused,
			"+", Ctrl + KeyUp, "Увеличить",
			"-", Ctrl + KeyDown, "Уменьшить");
		switch(result) {
		case 1:
			cmb_var = ev;
			execute(field_up); 
			break;
		case 2:
			cmb_var = ev;
			execute(field_down);
			break;
		}
	}
	if(pchoose) {
		if(addbutton(rc, focused, "...", KeyEnter, "Выбрать")) {
			cmb_var = ev;
			choose_proc = pchoose;
			draw::execute(execute_choose);
		}
	}
	auto a = area(rc);
	if(focused) {
		cedit.align = flags & edit_mask;
		cedit.update(ev, type, digits);
		cedit.view(rc);
	} else {
		char temp[260];
		auto p = getvalue(ev, type, temp, temp + sizeof(temp) / sizeof(temp[0]));
		draw::text(rc + metrics::edit, p, flags & edit_mask);
	}
}

int draw::field(int x, int y, int width, const char* header_label, const anyval& ev, int header_width, int digits) {
	draw::state push;
	setposition(x, y, width);
	if(header_label && header_label[0])
		titletext(x, y, width, 0, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	unsigned flags = AlignRight;
	if(isfocused(rc, ev))
		flags |= Focused;
	field(rc, flags | TextSingleLine, ev, digits, true, KindNumber, 0);
	return rc.height() + metrics::padding * 2;
}

int draw::field(int x, int y, int width, const char* header_label, const char*& sev, int header_width, eventproc pchoose) {
	draw::state push;
	setposition(x, y, width);
	if(header_label && header_label[0])
		titletext(x, y, width, 0, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	unsigned flags = AlignLeft;
	anyval av = sev;
	if(isfocused(rc, av))
		flags |= Focused;
	field(rc, flags | TextSingleLine, av, -1, false, KindText, pchoose);
	return rc.height() + metrics::padding * 2;
}

struct combolist : controls::list, adat<void*, 64> {
	const char* getname(char* result, const char* result_max, int line, int column) const override {
		return cmb_getname(data[line]);
	}
	static int compare_by_order(const void* v1, const void* v2) {
		auto p1 = *((void**)v1);
		auto p2 = *((void**)v2);
		auto n1 = cmb_getname(p1);
		auto n2 = cmb_getname(p2);
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
		qsort(data, getcount(), sizeof(data[0]), compare_by_order);
	}
	void choose(const void* value) {
		current = find(value);
	}
	void update() {
		clear();
		auto pe = cmb_source.end();
		for(auto pb = cmb_source.begin(); pb < pe; pb += cmb_source.size)
			add(pb);
		current = cmb_var.get();
	}
};

static void show_combolist() {
	combolist list;
	list.update();
	if(list.getcount() == 0)
		return;
	list.hilite_odd_lines = false;
	list.sort();
	list.pixels_per_line = list.getrowheight();
	list.lines_per_page = imin(list.getcount(), 7);
	rect rc = cmb_rect;
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
	list.choose(cmb_source.ptr(cmb_var.get()));
	list.ensurevisible();
	if(dropdown(rc, list)) {
		auto p = list.data[list.current];
		auto i = cmb_source.indexof(p);
		cmb_var.set(i);
	}
}

int draw::field(int x, int y, int width, const char* header_label, const anyval& av, int header_width, const acol& source, pgetname getname, const char* tips) {
	draw::state push;
	setposition(x, y, width);
	decortext(0);
	if(header_label && header_label[0])
		titletext(x, y, width, 0, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	if(rc.width() <= 0)
		return rc.height() + metrics::padding * 2;
	auto focused = isfocused(rc, av);
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
		case KeyEnter:
			execute_drop_down = true;
			break;
		}
	}
	if(execute_drop_down) {
		cmb_var = av;
		cmb_rect = rc;
		cmb_source = source;
		cmb_getname = getname;
		execute(show_combolist);
	}
	rco.offset(2, 2);
	if(focused)
		rectx(rco, colors::black);
	rco.offset(2, 2);
	auto v = av.get();
	auto p = source.ptr(v);
	textc(rco.x1, rco.y1, rco.width(), getname(p));
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc.height() + metrics::padding * 2;
}