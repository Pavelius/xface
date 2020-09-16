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
{"Identifier", {color::create(0, 0, 100)}},
{"Open parameters block", {color::create(255, 0, 255)}},
{"Close parameters block", {color::create(255, 0, 255)}},
{"Open code block", {color::create(255, 0, 255)}},
{"Close code block", {color::create(255, 0, 255)}},
{"Open scope block", {color::create(255, 0, 255)}},
{"Close scope block", {color::create(255, 0, 255)}},
{"Open user block", {color::create(0, 50, 100)}},
{"Close user block", {color::create(0, 50, 128)}},
};

void codeview::ensurevisible(int cw) {
	if(cw < 0 || cw >= maximum.y)
		return;
	if(cw < origin.y) {
		origin.y = cw;
		invalidate();
	}
	if(cw > origin.y + (lines_per_page - 1)) {
		origin.y = cw - (lines_per_page - 1);
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
	else if(strcmp(id, "parser") == 0)
		codemodel::set((const parseri*)value);
	else if(strcmp(id, "open") == 0)
		open((const char*)value);
	else if(strcmp(id, "select") == 0)
		set(value, false);
	else if(strcmp(id, "select_range") == 0)
		set(value, true);
}

void codeview::invalidate() {
	cash_origin = -1;
}

void codeview::redraw(const rect& rco) {
	draw::state push;
	draw::font = this->font;
	rect rc = rco + rctext;
	rcclient = rc;
	rc.y1 -= origin.y * fontsize.y;
	auto x = rc.x1, y = rc.y1;
	point pos = {};
	group_s type;
	auto ps = data;
	while(true) {
		auto x1 = x + pos.x*fontsize.x;
		auto y1 = y + pos.y*fontsize.y;
		auto c = getnext(ps, pos, type);
		if(!c)
			break;
		auto& ei = bsdata<groupi>::elements[type].visual;
		fore = ei.present;
		text(x1, y1, ps, c, ei.flags);
		ps += c;
	}
	// Draw hilite
	if(true) {
		auto x1 = x + pos1.x*fontsize.x;
		auto y1 = y + pos1.y*fontsize.y;
		line(x1, y1, x1, y1 + fontsize.y, colors::text.mix(colors::edit));
	}
	if(getbegin() != -1) {
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
		ensurevisible(pos1.y + 1);
		break;
	case KeyLeft:
	case KeyLeft | Shift:
	case KeyLeft | Ctrl:
	case KeyLeft | Shift | Ctrl:
		left((id&Shift) != 0, (id&Ctrl) != 0);
		ensurevisible(pos1.y - 1);
		break;
	case KeyUp:
	case KeyUp | Shift:
		if(getcurrentpos().y > 0) {
			auto pt = getcurrentpos();
			pt.y -= 1;
			set(getindex(pt), (id&Shift) != 0);
			ensurevisible(pt.y);
		}
		break;
	case KeyDown:
	case KeyDown | Shift:
		if(getcurrentpos().y < maximum.y) {
			auto pt = getcurrentpos();
			pt.y += 1;
			set(getindex(pt), (id&Shift) != 0);
			ensurevisible(pt.y);
		}
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
		}
		break;
	case KeyHome:
	case KeyHome | Shift:
		if(true) {
			auto i1 = lineb(p1);
			auto i2 = skipsp(i1);
			if(p1 == i1)
				break;
			if(p1 == i2)
				i2 = i1;
			set(i2, (id&Shift) != 0);
		}
		break;
	case KeyEnd:
	case KeyEnd | Shift:
		set(linee(getcurrent()), (id&Shift) != 0);
		break;
	default:
		return control::keyinput(id);
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
	case MouseLeft | Shift:
		if(hot.pressed) {
			point pt;
			pt.x = (hot.mouse.x - rcclient.x1 + fontsize.x / 2) / fontsize.x;
			pt.y = (hot.mouse.y - rcclient.y1) / fontsize.y + origin.y;
			auto i = getindex(pt);
			set(i, (id&Shift) != 0);
		}
		break;
	case MouseLeftDBL:
		if(hot.pressed) {
			left(false, true);
			right(true, true);
		}
		break;
	default:
		control::mouseinput(id, position);
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
	reserve(p1 + i2 + 1);
	memmove(data + p1 + i2, data + p1, (count - p1 + 1) * sizeof(char));
	memcpy(data + p1, input, i2); count += i2;
	invalidate();
	set(p1 + i2, false);
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
	invalidate();
}

void codeview::correction() {
	auto lenght = getlenght();
	if(p2 != -1 && p2 > lenght) {
		p2 = lenght;
		invalidate();
	}
	if(p1 > lenght) {
		p1 = lenght;
		invalidate();
	}
	if(p1 < 0) {
		p1 = 0;
		invalidate();
	}
}

void codeview::left(bool shift, bool ctrl) {
	const char* p = data + p1;
	if(!ctrl)
		p = nextstep(p, -1);
	else {
		while(p > data && iswhitespace(p[-1]))
			p = nextstep(p, -1);
		if(p > data) {
			auto result = isidentifier(p[-1]);
			while(p > data && isidentifier(p[-1]) == result)
				p = nextstep(p, -1);
		}
	}
	set(p - data, shift);
}

void codeview::right(bool shift, bool ctrl) {
	const char* p = data + p1;
	if(!ctrl)
		p = nextstep(p, 1);
	else {
		auto pe = data + count;
		while(p < pe && iswhitespace(*p))
			p = nextstep(p, 1);
		if(p < pe) {
			auto result = isidentifier(*p);
			while(p > data && isidentifier(*p) == result)
				p = nextstep(p, 1);
		}
	}
	set(p - data, shift);
}

codeview::codeview() : cash_origin(-1) {
}

void codeview::view(const rect& rc) {
	auto pixels_per_line = getpixelperline();
	if(!pixels_per_line)
		return;
	rcclient = rc;
	lines_per_page = rc.height() / pixels_per_line;
	if(cash_origin == -1) {
		getstate(p1, pos1, p2, pos2, size, {0, (short)origin.y}, cash_origin);
		maximum.x = size.x * fontsize.x;
		maximum.y = size.y;
	}
	auto screen_width = rc.width();
	auto maximum_x = maximum.x;
	auto enable_scrollh = maximum.x > screen_width;
	draw::scroll scrollv(origin.y, lines_per_page, maximum.y, rc);
	scrollv.input();
	control::view(rc);
	if(true) {
		draw::state push;
		setclip(rc);
		redraw(rc);
	}
	scrollv.view(isfocused());
	if(enable_scrollh)
		draw::scrollh({rc.x1, rc.y2 - metrics::scroll, rc.x2, rc.y2},
			origin.x, screen_width, maximum_x, isfocused());
}

const char* codeview::nextstep(const char* ps, int dir) {
	if(*ps == 10) {
		ps += dir;
		if(*ps == 13)
			ps += dir;
	} else if(*ps == 13) {
		ps += dir;
		if(*ps == 10)
			ps += dir;
	} else
		ps += dir;
	if(ps < data)
		return data;
	else if(ps >= (data + count - 1))
		return (data + count - 1);
	return ps;
}

control::command* codeview::getcommands() const {
	static command commands[] = {{"cut", "Вырезать", 0, &codeview::cut, -1, Ctrl + Alpha + 'X'},
	{"copy", "Копировать", 0, &codeview::copy, -1, Ctrl + Alpha + 'C'},
	{"paste", "Вставить", 0, &codeview::paste, -1, Ctrl + Alpha + 'V'},
	{}};
	return commands;
}

const sprite* codeview::font = (sprite*)loadb("art/fonts/code.pma");
point codeview::fontsize;