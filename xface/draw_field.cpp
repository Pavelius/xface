#include "crt.h"
#include "draw.h"
#include "draw_control.h"

using namespace draw;

static storage	edit_value;
static editproc edit_proc;

static struct edit_field_widget : controls::textedit {

	char	buffer[4192];
	storage	value;

	edit_field_widget() : textedit(buffer, sizeof(buffer) - 1, false) {
		buffer[0] = 0;
		show_border = false;
		show_background = false;
	}

	bool isfocused() const override {
		return (void*)getfocus() == value.data;
	}

	bool isfocusable() const override {
		return false;
	}

	void setfocus(bool instant) {
		draw::setfocus((int)value.data, instant);
	}

	void write() {
		if(value)
			value.set(buffer);
	}

	void read() {
		value.getf(buffer, zendof(buffer));
		select_all(true);
		invalidate();
	}

	void set(const storage& e) {
		if(value==e)
			return;
		write();
		value = e;
		read();
	}

} edit;

void draw::updatefocus() {
	if(edit.isfocused())
		edit.write();
}

static void loadfocus() {
	if(edit.isfocused())
		edit.read();
}

static void field_up() {
	edit.write();
	if(edit_value.type == storage::Number) {
		auto value = edit_value.get();
		edit_value.set(value + 1);
		loadfocus();
	}
}

static void field_down() {
	edit.write();
	if(edit_value.type == storage::Number) {
		auto value = edit_value.get();
		edit_value.set(value - 1);
		loadfocus();
	}
}

static void execute(callback proc, const storage& ev) {
	edit_value = ev;
	draw::execute(proc);
}

static void run_edit_proc() {
	edit_proc(edit_value);
	loadfocus();
}

static void execute(editproc proc, const storage& ev) {
	edit_value = ev;
	edit_proc = proc;
	draw::execute(run_edit_proc);
}

int draw::field(int x, int y, int width, unsigned flags, const storage& ev, const char* header_label, const char* tips, int header_width, editproc choose) {
	draw::state push;
	setposition(x, y, width);
	decortext(flags);
	if(header_label && header_label[0])
		titletext(x, y, width, flags, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	if(!isdisabled(flags))
		draw::rectf(rc, colors::window);
	focusing((int)ev.data, flags, rc);
	bool focused = isfocused(flags);
	draw::rectb(rc, colors::border);
	rect rco = rc;
	if(ev.type==storage::Number) {
		auto result = addbutton(rc, focused,
			"+", KeyUp, "Увеличить",
			"-", KeyDown, "Уменьшить");
		switch(result) {
		case 1: execute(field_up, ev);  break;
		case 2: execute(field_down, ev); break;
		}
	}
	if(choose) {
		if(addbutton(rc, focused, "...", F4, "Выбрать"))
			execute(choose, ev);
	}
	auto a = area(rc);
	if(isfocused(flags)) {
		edit.set(ev);
		edit.view(rc);
	} else {
		char temp[260];
		auto p = ev.get(temp, zendof(temp));
		draw::texte(rc + metrics::edit, p, flags, -1, -1);
	}
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc.height() + metrics::padding * 2;
}