#include "anyval.h"
#include "crt.h"
#include "draw.h"
#include "draw_control.h"

using namespace draw;

static anyval edit_value;

static const char* getvalue(const anyval& v, field_type_s t, char* result, const char* result_end) {
	if(!v)
		return "";
	const char* p;
	switch(t) {
	case FieldNumber:
		return szprint(result, result_end, "%1i", v.get());
	case FieldText:
		p = (const char*)v.get();
		if(p)
			return szprint(result, result_end, p);
		result[0] = 0;
		return result;
	default: return "";
	}
}

static void setvalue(const anyval& v, field_type_s t, const char* result) {
	if(!v)
		return;
	int value = 0;
	switch(t) {
	case FieldNumber: value = sz2num(result); break;
	case FieldText: value = (int)szdup(result); break;
	}
	v.set(value);
}

static class edit_driver : public controls::textedit {
	char			source[2048];
	anyval			value;
	field_type_s	type;
public:
	constexpr edit_driver() : textedit(source, sizeof(source) / sizeof(source), false),
		value(), source(), type(FieldNumber) {
	}
	bool isfocusable() const override {
		return false;
	}
	bool isfocused() const override {
		return getfocus() == (int)value.ptr();
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
	void update(const anyval& ev, field_type_s et, int digits = -1) {
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

void save_focus() {
	if(cedit.isfocused()) {
		cedit.save();
		cedit.clear();
	}
}

static void field_up() {
	edit_value.set(edit_value.get() + 1);
	cedit.load();
	cedit.invalidate();
}

static void field_down() {
	edit_value.set(edit_value.get() - 1);
	cedit.load();
	cedit.invalidate();
}

static void execute(callback proc, const anyval& ev) {
	edit_value = ev;
	draw::execute(proc);
}

void draw::field(const rect& rco, unsigned flags, const anyval& ev, int digits, field_type_s type) {
	rect rc = rco;
	draw::rectf(rc, colors::window);
	draw::rectb(rc, colors::border);
	auto focused = isfocused(flags);
	if(type == FieldNumber) {
		auto result = addbutton(rc, focused,
			"+", Ctrl + KeyUp, "Увеличить",
			"-", Ctrl + KeyDown, "Уменьшить");
		switch(result) {
		case 1: execute(field_up, ev);  break;
		case 2: execute(field_down, ev); break;
		}
	}
	auto a = area(rc);
	if(focused) {
		cedit.align = flags & AlignMask;
		cedit.update(ev, type, digits);
		cedit.view(rc);
	} else {
		char temp[260];
		auto p = getvalue(ev, type, temp, temp + sizeof(temp) / sizeof(temp[0]));
		draw::texte(rc + metrics::edit, p, flags & AlignMask, -1, -1);
	}
}

int draw::field(int x, int y, int width, const char* header_label, const anyval& ev, int header_width, int digits) {
	draw::state push;
	setposition(x, y, width);
	if(header_label && header_label[0])
		titletext(x, y, width, 0, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	unsigned flags = AlignRight;
	focusing((int)ev.ptr(), flags, rc);
	field(rc, flags, ev, digits, FieldNumber);
	return rc.height() + metrics::padding * 2;
}

int draw::field(int x, int y, int width, const char* header_label, const char*& sev, int header_width) {
	draw::state push;
	setposition(x, y, width);
	if(header_label && header_label[0])
		titletext(x, y, width, 0, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	unsigned flags = AlignLeft;
	focusing((int)&sev, flags, rc);
	field(rc, flags, anyval(sev), -1, FieldText);
	return rc.height() + metrics::padding * 2;
}