#include "codeview.h"
#include "setting.h"

using namespace draw;
using namespace draw::controls;

//IllegalSymbol,
//WhiteSpace, Operator, Keyword, Comment,
//Number, String, Identifier,
//OpenParam, CloseParam, OpenBlock, CloseBlock, OpenScope, CloseScope,

static int common_padding = 4;
static bool use_space_instead_tab = false;

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
	if(equal(id, "text"))
		codemodel::set((const char*)value);
	else if(equal(id, "lex"))
		codemodel::set((const lexer*)value);
	else if(equal(id, "parser"))
		codemodel::set((const parseri*)value);
	else if(equal(id, "open"))
		open((const char*)value);
	else if(equal(id, "select"))
		set(value, false);
	else if(equal(id, "select_range"))
		set(value, true);
}

void codeview::invalidate() {
	cash_origin = -1;
}

static void execute_wordselect() {
	auto p = (codeview*)hot.object;
	p->left(false, true);
	p->right(true, true);
}

void codeview::redraw(const rect& rco) {
	if(!fontsize.x || !fontsize.y)
		return;
	draw::state push;
	draw::font = this->font;
	rect rc = rco + rctext;
	// Mouse input handle
	rect r1 = rc;
	rc.y1 -= origin.y * fontsize.y;
	rc.x1 -= origin.x;
	auto x = rc.x1, y = rc.y1;
	if(ishilite(r1)) {
		point mpos;
		mpos.x = (hot.mouse.x - r1.x1 + origin.x + fontsize.x / 2) / fontsize.x;
		mpos.y = (hot.mouse.y - r1.y1) / fontsize.y + origin.y;
		auto x1 = rc.x1 + mpos.x * fontsize.x;
		auto y1 = rc.y1 + mpos.y * fontsize.y;
		rect r2 = {x1, y1, x1 + fontsize.x, y1 + fontsize.y};
		ishilite(r2);
		switch(hot.key) {
		case MouseLeft:
		case MouseLeft | Shift:
			if(hot.pressed) {
				auto i = getindex(mpos);
				if(hot.key&Shift)
					postsetvalue("select_range", i);
				else
					postsetvalue("select", i);
			}
			break;
		case MouseMove:
			if(hot.pressed) {
				auto i = getindex(mpos);
				postsetvalue("select_range", i);
			}
			break;
		case MouseLeftDBL:
			if(hot.pressed)
				draw::execute(execute_wordselect, 0, 0, this);
			break;
		}
	}
	point pos = {};
	group_s type;
	auto ps = begin();
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

void codeview::pastetab() {
}

void codeview::pastecr() {
	char temp[260];
	auto p = temp;
	auto pe = temp + sizeof(temp) - 1;
	auto n = lineb(p1);
	if(p < pe)
		*p++ = '\n';
	if(p < pe)
		*p++ = '\r';
	auto p1 = ptr(n);
	while(*p1 && iswhitespace(*p1)) {
		if(p < pe)
			*p++ = *p1;
		p1++;
	}
	//if(!isnextline(p1, 0)) {
	//	if(true) {
	//		for(auto i = 0; i < common_padding; i++) {
	//			if(p < pe)
	//				*p++ = ' ';
	//		}
	//	} else {
	//		if(p < pe)
	//			*p++ = '\t';
	//	}
	//}
	*p = 0;
	paste(temp);
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
	case KeyPageDown:
	case KeyPageDown | Shift:
		if(true) {
			auto pt = getcurrentpos();
			auto n = origin.y + lines_per_page - 1;
			if(pt.y != n)
				pt.y = n;
			else
				pt.y += lines_per_page - 1;
			set(getindex(pt), (id&Shift) != 0);
			ensurevisible(pt.y);
		}
		break;
	case KeyPageUp:
	case KeyPageUp | Shift:
		if(true) {
			auto pt = getcurrentpos();
			auto n = origin.y;
			if(pt.y != n)
				pt.y = n;
			else
				pt.y -= lines_per_page - 1;
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
	case KeyEnter:
		if(!readonly)
			pastecr();
		break;
	case KeyTab:
		if(!readonly)
			paste("\t");
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
		auto s1 = ptr(getbegin());
		auto s2 = ptr(getend());
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
	memmove(ptr(p1 + i2), ptr(p1), (count - p1 + 1) * sizeof(char));
	memcpy(ptr(p1), input, i2); count += i2;
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
	const char* p = ptr(p1);
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
	set(p - begin(), shift);
}

void codeview::right(bool shift, bool ctrl) {
	const char* p = ptr(p1);
	if(!ctrl)
		p = nextstep(p, 1);
	else {
		auto pe = end();
		while(p < pe && iswhitespace(*p))
			p = nextstep(p, 1);
		if(p < pe) {
			auto result = isidentifier(*p);
			while(p > data && isidentifier(*p) == result)
				p = nextstep(p, 1);
		}
	}
	set(p - begin(), shift);
}

codeview::codeview() : cash_origin(-1) {
}

void codeview::view(const rect& rc) {
	auto pixels_per_line = getpixelperline();
	if(!pixels_per_line)
		return;
	lines_per_page = rc.height() / pixels_per_line;
	if(cash_origin == -1) {
		getstate(p1, pos1, p2, pos2, size, {0, (short)origin.y}, cash_origin);
		maximum.x = size.x * fontsize.x;
		maximum.y = size.y;
	}
	draw::scroll scrollv(origin.y, lines_per_page, maximum.y, rc); scrollv.input();
	draw::scroll scrollh(origin.x, rc.width(), maximum.x, rc, true); scrollh.input();
	control::view(rc);
	if(true) {
		draw::state push;
		setclip(rc);
		redraw(rc);
	}
	scrollv.view(isfocused());
	scrollh.view(isfocused());
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
		return begin();
	else if(ps >= (end() - 1))
		return (end() - 1);
	return ps;
}

control::command* codeview::getcommands() const {
	static command commands[] = {{"cut", "��������", 0, &codeview::cut, -1, Ctrl + 'X'},
	{"copy", "����������", 0, &codeview::copy, -1, Ctrl + 'C'},
	{"paste", "��������", 0, &codeview::paste, -1, Ctrl + 'V'},
	{}};
	return commands;
}

const sprite* codeview::font = (sprite*)loadb("art/fonts/code.pma");
point codeview::fontsize;

static setting::element format_setting[] = {{"���������", {common_padding}},
{"������������ ������� ������ ���������", {use_space_instead_tab}},
};
static setting::header headers[] = {{"��������", "�����", "��������������", format_setting},
};