#include "anyval.h"
#include "crt.h"
#include "draw.h"
#include "draw_control.h"

using namespace draw;

static fnchoose		proc_choose;
static fntext		list_getname;
static fnallow		list_allow;
static rect			cmb_rect;
static array		cmb_source;
static anyval		cmb_var;
static const void*	cmb_param;

static const char* getvalue(const anyval& v, bool istext, stringbuilder& sb) {
	if(!v)
		return "";
	const char* p;
	if(istext) {
		if(v.getsize() == sizeof(const char*)) {
			p = (const char*)v.get();
			if(p)
				return p;
		} else
			return (const char*)v.getptr();
	} else {
		sb.add("%1i", v.get());
		return (const char*)(char*)sb;
	}
	return "";
}

static void setvalueg(const anyval& v, bool istext, const char* result) {
	if(!v)
		return;
	int value = 0;
	if(istext) {
		if(v.getsize() == sizeof(const char*))
			value = (int)szdup(result);
		else {
			unsigned len = zlen(result);
			if(len > v.getsize() - 1)
				len = v.getsize() - 1;
			auto p = (char*)v.getptr();
			memcpy(p, result, len);
			p[len] = 0;
		}
	} else
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
		return value && draw::isfocused(value);
	}
	void load() {
		stringbuilder sb(source);
		auto p = getvalue(value, istext, sb);
		if(p != source)
			zcpy(source, p, sizeof(source));
		invalidate();
	}
	void save() {
		setvalueg(value, istext, source);
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
	bool keyinput(unsigned id) override {
		switch(id) {
		case KeyEnter:
			if(!readonly) {
				save();
				select_all(true);
			}
			break;
		default:
			return textedit::keyinput(id);
		}
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
	array source;
	if(proc_choose(hot.object, source, cmb_var.getptr())) {
		cedit.load();
		cedit.select_all(true);
		cedit.invalidate();
	}
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

void draw::fieldf(const rect& rco, unsigned flags, const anyval& ev, int digits, bool increment, bool istext, fnchoose pchoose) {
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
		if(addbutton(rc, focused, "...", F4, "Выбрать")) {
			cmb_var = ev;
			proc_choose = pchoose;
			draw::execute(execute_choose, 0, 0, 0);
		}
	}
	if(focused) {
		cedit.align = flags & edit_mask;
		cedit.update(ev, istext, digits);
		cedit.view(rc);
	} else {
		char temp[260]; stringbuilder sb(temp);
		auto p = getvalue(ev, istext, sb);
		draw::text(rc + metrics::edit, p, flags & edit_mask);
	}
}

int draw::field(int x, int y, int width, const char* header_label, const anyval& ev, int header_width, int digits) {
	draw::state push;
	setposition(x, y, width);
	if(header_label && header_label[0])
		titletext(x, y, width, 0, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() - metrics::edit.height()};
	unsigned flags = AlignRight;
	if(isfocused(rc, ev))
		flags |= Focused;
	fieldf(rc, flags | TextSingleLine, ev, digits, true, false, 0);
	return rc.height() - metrics::edit.height();
}

int draw::field(int x, int y, int width, const char* header_label, char* sev, unsigned size, int header_width, fnchoose choosep) {
	draw::state push;
	setposition(x, y, width);
	if(header_label && header_label[0])
		titletext(x, y, width, 0, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() - metrics::edit.height()};
	unsigned flags = AlignLeft;
	anyval av;
	av.setvalue(sev, size);
	if(isfocused(rc, av))
		flags |= Focused;
	fieldf(rc, flags | TextSingleLine, av, -1, false, true, choosep);
	return rc.height() - metrics::edit.height();
}

int draw::field(int x, int y, int width, const char* header_label, const char*& sev, int header_width, fnchoose choosep) {
	draw::state push;
	setposition(x, y, width);
	if(header_label && header_label[0])
		titletext(x, y, width, 0, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() - metrics::edit.height()};
	unsigned flags = AlignLeft;
	anyval av = sev;
	if(isfocused(rc, av))
		flags |= Focused;
	fieldf(rc, flags | TextSingleLine, av, -1, false, true, choosep);
	return rc.height() - metrics::edit.height();
}

struct combolist : controls::list, reflist {
	const char* getname(stringbuilder& sb, int line, int column) const override {
		return list_getname(data[line], sb);
	}
	static int compare_by_order(const void* v1, const void* v2) {
		char t1[256]; stringbuilder sb1(t1);
		char t2[256]; stringbuilder sb2(t2);
		auto p1 = *((void**)v1);
		auto p2 = *((void**)v2);
		auto n1 = list_getname(p1, sb1);
		auto n2 = list_getname(p2, sb2);
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
			if(list_allow && !list_allow(cmb_param, (int)cmb_source.indexof(pb)))
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

void draw::fieldm(const rect& rc, const anyval& av, const array& source, fntext getname, bool instant, const void* param, fnallow allow) {
	cmb_var = av;
	cmb_rect = rc;
	cmb_source = source;
	cmb_param = param;
	list_getname = getname;
	list_allow = allow;
	if(instant)
		show_combolist();
	else
		execute(show_combolist);
}

void draw::fieldc(const rect& rc, const anyval& av, const array& source, fntext getname, const char* tips, const void* param, fnallow allow) {
	if(rc.width() <= 0)
		return;
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
	auto execute_drop_down = false;
	if(a && hot.key == MouseLeft && !hot.pressed)
		execute_drop_down = true;
	if(focused && hot.key == KeyEnter)
		execute_drop_down = true;
	if(execute_drop_down)
		fieldm(rc, av, source, getname, false, param, allow);
	rect rco = rc + metrics::edit;
	if(focused)
		rectx(rco, colors::black);
	auto v = av.get();
	if(av.getsize() != sizeof(void*))
		v = (int)source.ptr(v);
	char temp[260]; stringbuilder sb(temp);
	auto pn = "Отсутствует";
	if(v)
		pn = getname((void*)v, sb);
	if(pn)
		textc(rco.x1, rco.y1, rco.width(), pn);
	if(tips && a && !hot.pressed)
		tooltips(tips);
}

int draw::field(int x, int y, int width, const char* header_label, const anyval& av, int header_width, const array& source, fntext getname, const char* tips, const void* param, fnallow allow) {
	draw::state push;
	if(header_label && header_label[0]) {
		setposition(x, y, width);
		decortext(0);
		titletext(x, y, width, 0, header_label, header_width);
	}
	rect rc = {x, y, x + width, y + draw::texth() - metrics::edit.height()};
	if(rc.width() <= 0)
		return rc.height() - metrics::edit.height();
	fieldc(rc, av, source, getname, tips, param, allow);
	return rc.height() - metrics::edit.height();
}