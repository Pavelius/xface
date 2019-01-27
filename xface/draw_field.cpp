#include "crt.h"
#include "draw.h"
#include "draw_control.h"

using namespace draw;

static bool editstart(const rect& rc, unsigned flags, const cmdfd& callback_edit) {
	auto result = false;
	switch(hot.key&CommandMask) {
	case MouseMove:
	case InputIdle:
	case InputTimer:
	case KeyTab:
		// Команды не влияющие на вход в режим редактирования
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
		if(addbutton(rc, focused, ":dropdown", F4, "Показать список"))
			cmd.dropdown(rco, true);
	}
	if(cmd.choose(false)) {
		if(addbutton(rc, focused, "...", F4, "Выбрать значение"))
			cmd.choose(true);
	}
	if(cmd.increment(0, false)) {
		auto result = addbutton(rc, focused,
			"+", KeyUp, "Увеличить",
			"-", KeyDown, "Уменьшить");
		switch(result) {
		case 1: cmd.increment(+1, true); break;
		case 2: cmd.increment(-1, true); break;
		}
	}
	if(cmd.open(false)) {
		if(addbutton(rc, focused, "...", F4, "Выбрать"))
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
	char		buffer[4192];
	void*		value;
	unsigned	value_size;

	editfield() : textedit(buffer, sizeof(buffer) - 1, false), value(0), value_size(0) {
		buffer[0] = 0;
		show_border = false;
		show_background = false;
	}

	bool needfocus() const override {
		if(!getfocus())
			setfocus((int)value, true);
		return (void*)getfocus() == value;
	}

	void write(char* value, unsigned value_size) {
		if(!value)
			return;
	}

	void read(char* value, unsigned value_size) {
		if(this->value == value)
			return;
		write(value, value_size);
		this->value = value;
		this->value_size = value_size;
		if(value_size > sizeof(buffer) - 1)
			value_size = sizeof(buffer) - 1;
		zcpy(buffer, value, value_size);
		select_all(true);
	}

} edit;
}

int	draw::field(int x, int y, int width, unsigned flags, char* value, unsigned value_size, const char* header_label, const char* tips, int header_width) {
	draw::state push;
	setposition(x, y, width);
	decortext(flags);
	if(header_label && header_label[0])
		titletext(x, y, width, flags, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	if(!isdisabled(flags))
		draw::rectf(rc, colors::window);
	focusing((int)value, flags, rc);
	bool focused = isfocused(flags);
	draw::rectb(rc, colors::border);
	rect rco = rc;
	auto a = area(rc);
	if(isfocused(flags)) {
		edit.read(value, value_size);
		edit.view(rc);
	} else
		draw::texte(rc + metrics::edit, value, flags, -1, -1);
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc.height() + metrics::padding * 2;
}