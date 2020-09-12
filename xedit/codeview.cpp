#include "codeview.h"

using namespace draw;
using namespace draw::controls;

//IllegalSymbol,
//WhiteSpace, Operator, Keyword, Comment,
//Number, String, Identifier,
//OpenParam, CloseParam, OpenBlock, CloseBlock, OpenScope, CloseScope,

BSDATA(groupi) = {{"Illegal symbol", {color::create(255, 0, 0)}},
{"White space", {color::create(255, 255, 255)}},
{"Operator", {color::create(255, 128, 0)}},
{"Keyword", {color::create(0, 0, 255), TextBold}},
{"Comment", {color::create(0, 255, 0)}},
{"Number", {color::create(255, 255, 0)}},
{"String", {color::create(0, 255, 255)}},
{"Identifier", {color::create(0, 0, 0)}},
{"Open parameters block", {color::create(255, 0, 255)}},
{"Close parameters block", {color::create(255, 0, 255)}},
{"Open code block", {color::create(255, 0, 255)}},
{"Close code block", {color::create(255, 0, 255)}},
{"Open scope block", {color::create(255, 0, 255)}},
{"Close scope block", {color::create(255, 0, 255)}},
};

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
	else if(strcmp(id, "lex") == 0)
		set((const lexer*)value);
}

void codeview::invalidate() {
	cashed_width = -1;
}

void codeview::redraw(const rect& rco) {
	draw::state push;
	draw::font = this->font;
	rect rc = rco + rctext;
	rc.y1 -= origin.y;
	auto x = rc.x1, y = rc.y1;
	point size = {(short)textw('A'), (short)texth()};
	codepos cp = {};
	while(true) {
		auto x1 = x + cp.column*size.x;
		auto y1 = y + cp.line*size.y;
		getnext(cp);
		auto c = cp.count;
		if(!c)
			break;
		auto t = get(cp.from);
		auto& ei = bsdata<groupi>::elements[cp.type].visual;
		fore = ei.present;
		text(x1, y1, t, c, ei.flags);
		cp.from += cp.count;
	}
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

const sprite* codeview::font = (sprite*)loadb("art/fonts/code.pma");