#include "crt.h"
#include "draw.h"
#include "draw_control.h"
#include "storage.h"

using namespace draw;

static bool editstart(const rect& rc, unsigned flags, const cmdfd& callback_edit) {
	auto result = false;
	switch(hot.key&CommandMask) {
	case MouseMove:
	case InputIdle:
	case InputTimer:
	case KeyTab:
		// ������� �� �������� �� ���� � ����� ��������������
		break;
	case MouseLeft:
	case MouseLeftDBL:
	case MouseRight:
		result = draw::areb(rc);
		//if(result)
		//	execute(hot);
		break;
	case InputSymbol:
		result = true;
		break;
	default:
		result = (hot.key&CommandMask) >= KeyLeft;
		break;
	}
	if(result) {
		callback_edit.set(rc);
		callback_edit.execute();
	}
	return result;
}

int	draw::field(int x, int y, int width, unsigned flags, const cmdfd& cmd, const char* header_label, const char* tips, int header_width) {
	draw::state push;
	setposition(x, y, width);
	decortext(flags);
	if(header_label && header_label[0])
		titletext(x, y, width, flags, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	if(!isdisabled(flags))
		draw::rectf(rc, colors::window);
	focusing(cmd.getid(), flags, rc);
	bool focused = isfocused(flags);
	draw::rectb(rc, colors::border);
	rect rco = rc;
	if(cmd.dropdown(rco, false)) {
		if(addbutton(rc, focused, ":dropdown", F4, "�������� ������"))
			cmd.dropdown(rco, true);
	}
	if(cmd.choose(false)) {
		if(addbutton(rc, focused, "...", F4, "������� ��������"))
			cmd.choose(true);
	}
	if(cmd.increment(0, false)) {
		auto result = addbutton(rc, focused,
			"+", KeyUp, "���������",
			"-", KeyDown, "���������");
		switch(result) {
		case 1: cmd.increment(+1, true); break;
		case 2: cmd.increment(-1, true); break;
		}
	}
	if(cmd.open(false)) {
		if(addbutton(rc, focused, "...", F4, "�������"))
			cmd.open(true);
	}
	auto a = area(rc);
	bool enter_edit = false;
	if(focused)
		enter_edit = editstart(rc, flags, cmd);
	if(!enter_edit) {
		char temp[260];
		auto label = cmd.get(temp, zendof(temp), false);
		if(label) {
			if(isfocused(flags))
				draw::texte(rc + metrics::edit, label, flags, 0, zlen(label));
			else
				draw::texte(rc + metrics::edit, label, flags, -1, -1);
		}
		if(tips && a == AreaHilited)
			tooltips(tips);
	}
	return rc.height() + metrics::padding * 2;
}

namespace {

static struct editfield : controls::textedit {

	enum type_s : unsigned char { Text, TextPtr, Number };

	char		buffer[4192];
	storage		value;
	unsigned	value_size;

	editfield() : textedit(buffer, sizeof(buffer) - 1, false) {
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

	void write(const storage& e) {
		if(value)
			e.set(buffer);
	}

	void read(const storage& e) {
		if(value==e)
			return;
		write(value);
		value = e;
		e.getf(buffer, zendof(buffer));
		select_all(true);
		invalidate();
	}

} edit;
}

static int field_common(int x, int y, int width, unsigned flags, const storage& ev, const char* header_label, const char* tips, int header_width) {
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
	//rect rco = rc;
	auto a = area(rc);
	if(isfocused(flags)) {
		edit.read(ev);
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

int	draw::field(int x, int y, int width, unsigned flags, char* value, unsigned value_size, const char* header_label, const char* tips, int header_width) {
	return field_common(x, y, width, flags,
		storage(storage::Text, value, value_size),
		header_label, tips, header_width);
}

int	draw::field(int x, int y, int width, unsigned flags, const char** value, const char* header_label, const char* tips, int header_width) {
	return field_common(x, y, width, flags,
		storage(storage::TextPtr, value, 4),
		header_label, tips, header_width);
}

int	draw::field(int x, int y, int width, unsigned flags, int& number, const char* header_label, const char* tips, int header_width) {
	return field_common(x, y, width, flags, storage(number),
		header_label, tips, header_width);
}

int	draw::field(int x, int y, int width, unsigned flags, short& number, const char* header_label, const char* tips, int header_width) {
	return field_common(x, y, width, flags, storage(number),
		header_label, tips, header_width);
}