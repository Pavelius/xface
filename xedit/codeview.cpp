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
{"Keyword", {color::create(0, 0, 128), TextBold}},
{"Comment", {color::create(0, 128, 0)}},
{"Number", {color::create(128, 128, 0)}},
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
	if(linenumber == -1)
		return;
	auto cw = fontsize.y*getcurrent();
	if(cw < origin.y) {
		origin.y = cw;
		invalidate();
	}
	if(getcurrent() > origin.y + (lines_per_page - 1)*fontsize.y) {
		origin.y = cw - (lines_per_page + 1)*fontsize.y;
		invalidate();
	}
}

int	codeview::linen(int index) const {
	return 0;
}

int	codeview::hittest(rect rc, point pt, unsigned state) const {
	return draw::hittest({rc.x1, rc.y1 - origin.y, rc.x2, rc.y2}, get(0), state, pt);
}

void codeview::open(const char* url) {
	codemodel::set(loadt(url));
	invalidate();
}

void codeview::setvalue(const char* id, int value) {
	if(strcmp(id, "text") == 0)
		codemodel::set((const char*)value);
	else if(strcmp(id, "lex") == 0)
		codemodel::set((const lexer*)value);
	else if(strcmp(id, "open") == 0)
		open((const char*)value);
	else if(strcmp(id, "select") == 0)
		set(value, false);
	else if(strcmp(id, "select_range") == 0)
		set(value, true);
}

void codeview::invalidate() {
	wheels.y = fontsize.y;
	cash_columns = -1;
}

void codeview::beforeredraw(const rect& rc) {
	if(!fontsize.y)
		return;
	lines_per_page = rc.height() / fontsize.y;
	if(cash_columns == -1) {
		updatestate();
		maximum.x = size.x * fontsize.x;
		maximum.y = size.y * fontsize.y;
	}
}

void codeview::redraw(const rect& rco) {
	draw::state push;
	draw::font = this->font;
	rect rc = rco + rctext;
	rc.y1 -= origin.y;
	auto x = rc.x1, y = rc.y1;
	codepos cp = {};
	while(true) {
		auto x1 = x + cp.column*fontsize.x;
		auto y1 = y + cp.line*fontsize.y;
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
	// Draw hilite
	if(getbegin() == getend()) {
		auto p1 = getbeginpos();
		auto x1 = x + p1.x*fontsize.x;
		auto y1 = y + p1.y*fontsize.y;
		line(x1, y1, x1, y1 + fontsize.y, colors::text.mix(colors::edit));
	} else if(getbegin() != -1) {
		auto p1 = getbeginpos();
		auto p2 = getendpos();
		for(auto i = p1.y; i <= p2.y; i++) {
			auto x1 = rco.x1;
			auto x2 = rco.x2;
			auto y1 = y + i * fontsize.y;
			if(i == p1.y)
				x1 = x + p1.x*fontsize.x;
			if(i == p2.y)
				x2 = x + p2.x*fontsize.x;
			rectf({x1, y1, x2, y1 + fontsize.y}, colors::edit, 64);
		}
	}
}

bool codeview::keyinput(unsigned id) {
	switch(id) {
	case KeyRight:
	case KeyRight | Shift:
	case KeyRight | Ctrl:
	case KeyRight | Shift | Ctrl:
		right((id&Shift) != 0, (id&Ctrl) != 0);
		ensurevisible(p1);
		break;
	case KeyLeft:
	case KeyLeft | Shift:
	case KeyLeft | Ctrl:
	case KeyLeft | Shift | Ctrl:
		left((id&Shift) != 0, (id&Ctrl) != 0);
		ensurevisible(p1);
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
		if(true) {
			auto i0 = getcurrent();
			auto i1 = lineb(i0);
			auto i2 = skipsp(i1);
			if(i0 == i1)
				break;
			if(i0 == i2)
				i2 = i1;
			set(i2, (id&Shift) != 0);
		}
		break;
	case KeyEnd:
	case KeyEnd | Shift:
		set(linee(getcurrent()), (id&Shift) != 0);
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

point codeview::getbeginpos() const {
	if(p2 == -1)
		return pos1;
	if(p1 < p2)
		return pos1;
	return pos2;
}

int	codeview::getbegin() const {
	if(p2 == -1)
		return p1;
	return imin(p1, p2);
}

int	codeview::getend() const {
	if(p2 == -1)
		return p1;
	return imax(p1, p2);
}

point codeview::getendpos() const {
	if(p2 == -1)
		return pos1;
	if(p1 > p2)
		return pos1;
	return pos2;
}

void codeview::clear() {
	if(p2 != -1 && p1 != p2 && data) {
		auto s1 = data + getbegin();
		auto s2 = data + getend();
		while(*s2)
			*s1++ = *s2++;
		*s1 = 0;
		invalidate();
		if(p1 > p2)
			p1 = p2;
	}
	p2 = -1;
}

void codeview::paste(const char* input) {
	clear();
	auto i2 = zlen(input);
	auto count = getlenght();
	if(p1 + i2 > count)
		reserve(p1 + i2 + 1);
	memmove(data + p1 + i2, data + p1, (count - p1 + 1) * sizeof(char));
	memcpy(data + p1, input, i2); count += i2;
	invalidate();
	set(p1 + i2, false);
}

void codeview::textout(int x, int y, int start) {
	codepos pos = {};
	getnext(pos);
}

void codeview::instance() {
	auto old_font = draw::font;
	draw::font = codeview::font;
	fontsize.x = textw('A');
	fontsize.y = texth();
	draw::font = old_font;
}

void codeview::set(int index, bool shift) {
	if(index < 0)
		index = 0;
	else if(index > getlenght())
		index = count;
	if(shift) {
		if(p2 == -1)
			p2 = p1;
	} else
		p2 = -1;
	p1 = index;
}

void codeview::correction() {
	auto lenght = getlenght();
	if(p2 != -1 && p2 > lenght)
		p2 = lenght;
	if(p1 > lenght)
		p1 = lenght;
	if(p1 < 0)
		p1 = 0;
}

void codeview::left(bool shift, bool ctrl) {
	auto p = data + p1;
	if(!ctrl)
		p--;
	else {
		for(; p > data && iswhitespace(p[-1]); p--);
		for(; p > data && !iswhitespace(p[-1]); p--);
	}
	set(p - data, shift);
}

void codeview::right(bool shift, bool ctrl) {
	auto n = p1;
	if(!ctrl)
		n += 1;
	else {
		n = skipnsp(n);
		n = skipsp(n);
	}
	set(n, shift);
}

point codeview::getpos(int index) const {
	point result, result2, size;
	int index2 = -1;
	getstate(index, result, index2, result2, size);
	return result;
}

codeview::codeview() : cash_columns(-1) {
}

control::command* codeview::getcommands() const {
	static command commands[] = {{"cut", "��������", 0, &codeview::cut, -1, Ctrl + Alpha + 'X'},
	{"copy", "����������", 0, &codeview::copy, -1, Ctrl + Alpha + 'C'},
	{"paste", "��������", 0, &codeview::paste, -1, Ctrl + Alpha + 'V'},
	{}};
	return commands;
}

const sprite* codeview::font = (sprite*)loadb("art/fonts/code.pma");
point codeview::fontsize;