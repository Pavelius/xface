#include "codeview.h"

using namespace draw;
using namespace draw::controls;

void codeview::ensurevisible(int linenumber) {
	// TODO: Позиционировать строку, чтобы она была видна.
	// 1) Вычислить общее количество строк.
	// 2) Вычислить количество строк на экране.
}

int	codeview::lineb(int index) const {
	return 0;
}

int	codeview::linee(int index) const {
	return 0;
}

int	codeview::linen(int index) const {
	return 0;
}

point codeview::getpos(rect rc, int index, unsigned state) const {
	auto line_number = 0;
	auto line_count = 0;
	auto line_start = draw::textlb(get(0), index, rc.width(), &line_number, &line_count);
	auto p = get(line_start);
	auto x1 = aligned(rc.x1, rc.width(), state, textw(p, line_count));
	auto y1 = rc.y1 + draw::alignedh(rc, get(0), state);
	return{
		(short)(x1 + textw(get(line_start), index - line_start)),
		(short)(y1 + line_number * texth() - origin.y)
	};
}

int	codeview::hittest(rect rc, point pt, unsigned state) const {
	return draw::hittest({rc.x1, rc.y1 - origin.y, rc.x2, rc.y2}, get(0), state, pt);
}

void codeview::setvalue(const char* id, int value) {
	if(strcmp(id, "text") == 0)
		set((const char*)value);
}

void codeview::invalidate() {
	cashed_width = -1;
}

void codeview::redraw(const rect& rcorigin) {
	rect rc = rcorigin + rctext;
	if(show_border)
		rc.offset(-1, -1);
	rc.y1 -= origin.y;
	text(rc, get(0), 0, 0);
}

bool codeview::keyinput(unsigned id) {
	switch(id) {
	case KeyRight:
	case KeyRight | Shift:
	case KeyRight | Ctrl:
	case KeyRight | Shift | Ctrl:
		right((id&Shift) != 0, (id&Ctrl) != 0);
		break;
	case KeyLeft:
	case KeyLeft | Shift:
	case KeyLeft | Ctrl:
	case KeyLeft | Shift | Ctrl:
		left((id&Shift) != 0, (id&Ctrl) != 0);
		break;
	case KeyBackspace:
		if(!readonly) {
			if(!isselected())
				set(getcurrent() - 1, true);
			clear();
		}
		break;
	case KeyDelete:
		if(!readonly) {
			if(!isselected())
				set(getcurrent() + 1, true);
			clear();
		}
		break;
	case InputSymbol:
		if(hot.param >= 0x20 && !readonly) {
			char temp[8];
			paste(szput(temp, hot.param));
			invalidate();
		}
		break;
	case KeyHome:
	case KeyHome | Shift:
		//select(lineb(p1), (id&Shift) != 0);
		break;
	case KeyEnd:
	case KeyEnd | Shift:
		//select(linee(p1), (id&Shift) != 0);
		break;
	default:
		return scrollable::keyinput(id);
	}
	return true;
}

unsigned codeview::select_all(bool run) {
	if(run) {
		set(0, false);
		set(getlenght(), true);
	}
	return 0;
}

bool codeview::copy(bool run) {
	if(!isselected())
		return false;
	if(run) {
		auto s1 = get(getbegin());
		auto s2 = get(getend());
		clipboard::copy(s1, s2 - s1);
	}
	return true;
}

bool codeview::paste(bool run) {
	if(readonly)
		return false;
	if(run) {
		clear();
		auto p = clipboard::paste();
		if(p)
			paste(p);
		delete p;
	}
	return true;
}

void codeview::mouseinput(unsigned id, point position) {
	switch(id) {
	case MouseLeft:
		if(hot.pressed) {
			auto i = hittest(rcclient, position, 0);
			if(i == -2)
				set(0, false);
			else if(i == -3)
				set(getlenght(), false);
			else if(i >= 0)
				set(i, false);
		}
		break;
	default:
		scrollable::mouseinput(id, position);
		break;
	}
}

bool codeview::cut(bool run) {
	return true;
}

void codeview::textout(int x, int y, int start) {
	codepos pos = {};
	getnext(pos);
}

control::command* codeview::getcommands() const {
	static command commands[] = {{"cut", "Вырезать", 0, &codeview::cut, -1, Ctrl + Alpha + 'X'},
	{"copy", "Копировать", 0, &codeview::copy, -1, Ctrl + Alpha + 'C'},
	{"paste", "Вставить", 0, &codeview::paste, -1, Ctrl + Alpha + 'V'},
	{}};
	return commands;
}