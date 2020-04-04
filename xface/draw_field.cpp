#include "anyval.h"
#include "crt.h"
#include "draw.h"
#include "draw_control.h"

using namespace draw;

static fnchoose		proc_choose;
static rect			cmb_rect;
static array		cmb_source;
static anyval		cmb_var;
static const void*	cmb_param;
static fntext		cmb_getname;
static fnallow		cmd_allow;

static const char* getvalue(const anyval& v, bool istext, char* result, const char* result_end) {
	if(!v)
		return "";
	const char* p;
	if(istext) {
		p = (const char*)v.get();
		if(p)
			return szprint(result, result_end, p);
		result[0] = 0;
	} else
		return szprint(result, result_end, "%1i", v.get());
	return result;
}

static void setvalue(const anyval& v, bool istext, const char* result) {
	if(!v)
		return;
	int value = 0;
	if(istext)
		value = (int)szdup(result);
	else
		value = sz2num(result);
	v.set(value);
}

static class edit_driver : public controls::textedit {
	anyval			value;
	char			source[2048];
	bool			istext;
public:
	edit_driver() : textedit(source, sizeof(source) / sizeof(source), false), value(), source(), istext(false) {}
	bool isfocusable() const override {
		return false;
	}
	bool isfocused() const override {
		return draw::isfocused(value);
	}
	void load() {
		auto p = getvalue(value, istext, source, source + sizeof(source) / sizeof(source[0]) - 1);
		if(p != source)
			zcpy(source, p, sizeof(source));
		invalidate();
	}
	void save() {
		setvalue(value, istext, source);
	}
	void update(const anyval& ev, bool istext, int digits = -1) {
		if(this->value == ev)
			return;
		this->value = ev;
		this->istext = istext;
		if(digits == -1)
			setcount(sizeof(source) / sizeof(source[0]) - 1);
		else
			setcount(digits);
		load();
		select_all(true);
	}
	void* getsource() {
		return value.getptr();
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
	if(!proc_choose)
		return;
	proc_choose(cmb_var);
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

void draw::field(const rect& rco, unsigned flags, const anyval& ev, int digits, bool increment, bool istext, fnchoose pchoose) {
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
			proc_choose = pchoose;
			draw::execute(execute_choose);
		}
	}
	if(focused) {
		cedit.align = flags & edit_mask;
		cedit.update(ev, istext, digits);
		cedit.view(rc);
	} else {
		char temp[260];
		auto p = getvalue(ev, istext, temp, temp + sizeof(temp) / sizeof(temp[0]));
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
	field(rc, flags | TextSingleLine, ev, digits, true, false, 0);
	return rc.height() + metrics::padding * 2;
}

int draw::field(int x, int y, int width, const char* header_label, const char*& sev, int header_width, fnchoose choosep) {
	draw::state push;
	setposition(x, y, width);
	if(header_label && header_label[0])
		titletext(x, y, width, 0, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	unsigned flags = AlignLeft;
	anyval av = sev;
	if(isfocused(rc, av))
		flags |= Focused;
	field(rc, flags | TextSingleLine, av, -1, false, true, choosep);
	return rc.height() + metrics::padding * 2;
}

struct combolist : controls::list, reflist {
	const char* getname(char* result, const char* result_max, int line, int column) const override {
		return cmb_getname(data[line], result, result_max);
	}
	static int compare_by_order(const void* v1, const void* v2) {
		char t1[256];
		char t2[256];
		auto p1 = *((void**)v1);
		auto p2 = *((void**)v2);
		auto n1 = cmb_getname(p1, t1, t1 + sizeof(t1) - 1);
		auto n2 = cmb_getname(p2, t2, t2 + sizeof(t2) - 1);
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
	static int getvalue(const anyval& var, const array& source) {
		auto v = var.get();
		if(var.getsize() != sizeof(void*))
			v = (int)source.ptr(v);
		return v;
	}
	void update() {
		clear();
		auto pe = cmb_source.end();
		for(auto pb = cmb_source.begin(); pb < pe; pb += cmb_source.getsize()) {
			if(cmd_allow && !cmd_allow(cmb_param, (int)pb))
				continue;
			add(pb);
		}
		current = getvalue(cmb_var, cmb_source);
	}
};

static void show_combolist() {
	combolist list;
	list.drop_shadow = true;
	list.update();
	if(list.getcount() == 0)
		return;
	list.hilite_odd_lines = false;
	list.sort();
	list.pixels_per_line = list.getrowheight();
	list.lines_per_page = imin(list.getcount(), 7);
	rect rc = cmb_rect;
	rc.x1 = rc.x1 - 1;
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
		if(cmb_var.getsize() != sizeof(void*)) {
			auto i = cmb_source.indexof(p);
			cmb_var.set(i);
		} else
			cmb_var.set((int)p);
	}
}

void draw::field(const rect& rc, const anyval& av, const array& source, fntext getname, bool instant, const void* param, fnallow allow) {
	cmb_var = av;
	cmb_rect = rc;
	cmb_source = source;
	cmb_param = param;
	cmb_getname = getname;
	cmd_allow = allow;
	if(instant)
		show_combolist();
	else
		execute(show_combolist);
}

int draw::field(int x, int y, int width, const char* header_label, const anyval& av, int header_width, const array& source, fntext getname, const char* tips, const void* param, fnallow allow) {
	draw::state push;
	if(header_label && header_label[0]) {
		setposition(x, y, width);
		decortext(0);
		titletext(x, y, width, 0, header_label, header_width);
	}
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	if(rc.width() <= 0)
		return rc.height() + metrics::padding * 2;
	auto focused = isfocused(rc, av);
	auto a = ishilite(rc);
	color active = colors::button.mix(colors::edit, 128);
	if(a) {
		if(hot.pressed)
			gradv(rc, active.darken(), active.lighten());
		else
			gradv(rc, active.lighten(), active.darken());
	} else
		gradv(rc, colors::button.lighten(), colors::button.darken());
	rectb(rc, colors::border);
	rect rco = rc;
	auto execute_drop_down = false;
	if(a && hot.key == MouseLeft && !hot.pressed)
		execute_drop_down = true;
	if(focused && hot.key==KeyEnter)
		execute_drop_down = true;
	if(execute_drop_down)
		field(rc, av, source, getname, false, param, allow);
	rco.offset(2, 2);
	if(focused)
		rectx(rco, colors::black);
	rco.offset(2, 2);
	auto v = av.get();
	if(av.getsize() != sizeof(void*))
		v = (int)source.ptr(v);
	char temp[261]; auto pn = getname((void*)v, temp, temp + sizeof(temp) - 1);
	if(pn)
		textc(rco.x1, rco.y1, rco.width(), pn);
	if(tips && a && !hot.pressed)
		tooltips(tips);
	return rc.height() + metrics::padding * 2;
}