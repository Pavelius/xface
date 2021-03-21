#include "draw_control.h"
#include "lexer.h"
#include "package.h"

using namespace draw;
using namespace draw::controls;

struct groupi {
	const char*			name;
	color				present;
	unsigned			flags;
};
BSDATA(groupi) = {{"Illegal symbol", {color::create(255, 0, 0)}},
	{"White space", {color::create(255, 255, 255)}},
	{"Comment", {color::create(0, 128, 0)}},
	{"Operator", {color::create(255, 128, 0)}},
	{"Keyword", {color::create(0, 0, 128)}, TextBold},
	{"Number", {color::create(128, 128, 0)}},
	{"String", {color::create(128, 0, 255)}},
	{"Identifier", {color::create(0, 0, 0)}},
	{"Type", {color::create(0, 0, 100)}},
};
assert_enum(groupi, Type)

static const sprite*	fontedit = (sprite*)loadb("art/fonts/code.pma");
static point			fontsize;
static lexer*			lex;
static const char*		line_feed = "\r\n";

class widget_editor : public control, vector<char> {
	const char*			url = 0;
	int					cash_origin = -1;
	int					lines_per_page = 0;
	int					p1 = 0, p2 = 0;
	int					horiz_position = -1;
	pointl				pos1 = {}, pos2 = {}, size = {};
	pointl				origin = {}, maximum = {};
	rect				rctext = {4, 4, 4, 4};
	bool				readonly = false;
	bool				modified = false;
	bool ismodified() const {
		return modified;
	}
	void ensurevisible(int v) {
		if(v < 0 || v >= maximum.y)
			return;
		if(v < origin.y) {
			origin.y = v;
			invalidate();
		}
		if(v > origin.y + (lines_per_page - 1)) {
			origin.y = v - (lines_per_page - 1);
			invalidate();
		}
	}
	void invalidate() {
		cash_origin = -1;
	}
	static bool iswhitespace(char s) {
		return s == 9 || s == 32 || s == 10 || s == 13;
	}
	static bool isidentifier(char sym) {
		return ischa(sym) || sym == '_' || (sym >= '0' && sym <= '9');
	}
	static const char* next(const char* p) {
		if(*p == 10) {
			p++;
			if(*p == 13)
				p++;
		} else if(*p == 13) {
			p++;
			if(*p == 10)
				p++;
		} else if(*p)
			p++;
		return p;
	}
	static const char* prev(const char* pb, const char* p) {
		if(p <= pb)
			return p;
		else if(p[-1] == 10) {
			p--;
			if(p > pb && p[-1] == 13)
				p--;
		} else if(p[-1] == 13) {
			p--;
			if(p > pb && p[-1] == 10)
				p--;
		} else
			p--;
		return p;
	}
	static const char* linee(const char* p) {
		while(*p && *p != 10 && *p != 13)
			p++;
		return p;
	}
	static const char* lineb(const char* pb, const char* p) {
		while(p > pb && p[-1] != 10 && p[-1] != 13)
			p--;
		return p;
	}
	int	hittest(rect rc, point pt, unsigned state) const {
		return draw::hittest({rc.x1, rc.y1 - origin.y, rc.x2, rc.y2}, ptr(0), state, pt);
	}
	const char* getcurrent() const {
		return (char*)data + p1;
	}
	const char* getstart() const {
		return (char*)data;
	}
	int	getbegin() const {
		if(p2 == -1)
			return p1;
		return imin(p1, p2);
	}
	int	getend() const {
		if(p2 == -1)
			return p1;
		return imax(p1, p2);
	}
	pointl getbeginpos() const {
		if(p2 == -1)
			return pos1;
		if(p1 < p2)
			return pos1;
		return pos2;
	}
	pointl getendpos() const {
		if(p2 == -1)
			return pos1;
		if(p1 > p2)
			return pos1;
		return pos2;
	}
	pointl getcurrentpos() const {
		if(horiz_position != -1)
			return {horiz_position, pos1.y};
		return pos1;
	}
	int	getindex(pointl pt) const {
		pointl pt1, pt2, size;
		int p1 = -1, p2 = -1, r3;
		lexer::get(begin(), p1, pt1, p2, pt2, size, pt, r3);
		return r3;
	}
	bool isselected() const {
		return p2 != -1 && p1 != -1;
	}
	static void execute_wordselect() {
		auto p = (widget_editor*)hot.object;
		p->left(false, true);
		p->right(true, true);
	}
	void setvalue(const char* id, int value) override {
		if(equal(id, "open"))
			open((const char*)value);
		else if(equal(id, "select"))
			set(value, false);
		else if(equal(id, "select_range"))
			set(value, true);
	}
	void redraw(const rect& rco) {
		if(!fontsize.x || !fontsize.y)
			return;
		draw::state push;
		draw::font = fontedit;
		rect rc = rco + rctext;
		// Mouse input handle
		rect r1 = rc;
		rc.y1 -= origin.y * fontsize.y;
		rc.x1 -= origin.x;
		auto x = rc.x1, y = rc.y1;
		if(ishilite(r1)) {
			pointl mpos;
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
					if(hot.key & Shift)
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
		pointl pos = {};
		group_s type;
		const char* ps = begin();
		while(true) {
			auto x1 = x + pos.x * fontsize.x;
			auto y1 = y + pos.y * fontsize.y;
			auto pb = ps;
			ps = lex->next(ps, pos, type);
			if(pb == ps)
				break;
			auto& ei = bsdata<groupi>::elements[type];
			if(type != WhiteSpace) {
				fore = ei.present;
				text(x1, y1, pb, ps - pb, ei.flags);
			}
		}
		// Draw hilite
		if(isfocused()) {
			auto x1 = x + pos1.x * fontsize.x;
			auto y1 = y + pos1.y * fontsize.y;
			line(x1, y1, x1, y1 + fontsize.y, colors::text.mix(colors::edit));
		}
		auto draw_selecton = getbegin() != -1;
		if(draw_selecton) {
			auto p1 = getbeginpos();
			auto p2 = getendpos();
			for(auto i = p1.y; i <= p2.y; i++) {
				auto x1 = rco.x1;
				auto x2 = rco.x2;
				auto y1 = y + i * fontsize.y;
				if(i == p1.y)
					x1 = x + p1.x * fontsize.x;
				if(i == p2.y)
					x2 = x + p2.x * fontsize.x;
				rectf({x1, y1, x2, y1 + fontsize.y}, colors::edit, 128);
			}
		}
	}
	int	getpixelperline() const {
		return fontsize.y;
	}
	void view(const rect& rc) {
		auto pixels_per_line = getpixelperline();
		if(!pixels_per_line)
			return;
		lines_per_page = rc.height() / pixels_per_line;
		if(cash_origin == -1) {
			lexer::get(begin(), p1, pos1, p2, pos2, size, {0, (short)origin.y}, cash_origin);
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
	int getlenght() const {
		return getcount();
	}
	void set(int index, bool shift, bool keephoriz = false) {
		if(index < 0)
			index = 0;
		else if(index > getlenght())
			index = getcount();
		if(shift) {
			if(p2 == -1)
				p2 = p1;
		} else
			p2 = -1;
		p1 = index;
		if(keephoriz) {
			if(horiz_position == -1)
				horiz_position = pos1.x;
		} else
			horiz_position = -1;
		invalidate();
	}
	void correction() {
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
	void clear() {
		if(p2 != -1 && p1 != p2 && data) {
			auto s1 = ptr(getbegin());
			auto s2 = ptr(getend());
			while(*s2)
				*s1++ = *s2++;
			*s1 = 0;
			invalidate();
			if(p1 > p2)
				p1 = p2;
			modified = true;
		}
		p2 = -1;
	}
	void paste(const char* input) {
		clear();
		auto i2 = zlen(input);
		reserve(p1 + i2 + 1);
		memmove(ptr(p1 + i2), ptr(p1), (count - p1 + 1) * sizeof(char));
		memcpy(ptr(p1), input, i2); count += i2;
		invalidate();
		set(p1 + i2, false);
		modified = true;
	}
	void pastecr() {
		char temp[260]; stringbuilder sb(temp);
		sb.add(line_feed);
		auto p = lineb(begin(), begin() + p1);
		while(*p==32 || *p==9)
			sb.add(*p++);
		paste(temp);
	}
	void left(bool shift, bool ctrl) {
		auto pb = getstart();
		auto p = getcurrent();
		if(!ctrl)
			p = prev(pb, p);
		else if(p > pb) {
			if(iswhitespace(p[-1])) {
				while(p > pb && iswhitespace(p[-1]))
					p = prev(pb, p);
			} else {
				auto result = isidentifier(p[-1]);
				while(p > data && !iswhitespace(p[-1]) && isidentifier(p[-1]) == result)
					p = prev(pb, p);
			}
		}
		set(p - begin(), shift);
	}
	void right(bool shift, bool ctrl) {
		const char* p = ptr(p1);
		if(!ctrl)
			p = next(p);
		else {
			auto pe = end();
			if(iswhitespace(*p)) {
				while(p < pe && iswhitespace(*p))
					p = next(p);
			} else {
				auto result = isidentifier(*p);
				while(p < pe && !iswhitespace(*p) && isidentifier(*p) == result)
					p = next(p);
			}
		}
		set(p - begin(), shift);
	}
	bool keyinput(unsigned id) override {
		switch(id) {
		case KeyRight:
		case KeyRight | Shift:
		case KeyRight | Ctrl:
		case KeyRight | Shift | Ctrl:
			right((id & Shift) != 0, (id & Ctrl) != 0);
			ensurevisible(pos1.y + 1);
			break;
		case KeyLeft:
		case KeyLeft | Shift:
		case KeyLeft | Ctrl:
		case KeyLeft | Shift | Ctrl:
			left((id & Shift) != 0, (id & Ctrl) != 0);
			ensurevisible(pos1.y - 1);
			break;
		case KeyUp:
		case KeyUp | Shift:
			if(getcurrentpos().y > 0) {
				auto pt = getcurrentpos();
				pt.y -= 1;
				set(getindex(pt), (id & Shift) != 0, true);
				ensurevisible(pt.y);
			}
			break;
		case KeyDown:
		case KeyDown | Shift:
			if(getcurrentpos().y < maximum.y) {
				auto pt = getcurrentpos();
				pt.y += 1;
				set(getindex(pt), (id & Shift) != 0, true);
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
				set(getindex(pt), (id & Shift) != 0, true);
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
				set(getindex(pt), (id & Shift) != 0, true);
				ensurevisible(pt.y);
			}
			break;
		case KeyBackspace:
			if(!readonly) {
				if(!isselected()) {
					auto pb = getstart();
					auto ps = getcurrent();
					set(prev(pb, ps) - pb, true);
				}
				clear();
			}
			break;
		case KeyDelete:
			if(!readonly) {
				if(!isselected()) {
					auto pb = getstart();
					auto ps = getcurrent();
					set(next(ps) - pb, true);
				}
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
				auto pb = getstart();
				auto ps = getcurrent();
				auto pa = lineb(pb, ps);
				auto pp = skipsp(pa);
				if(ps == pa)
					break;
				if(ps == pp)
					ps = pa;
				else
					ps = pp;
				set(ps - pb, (id & Shift) != 0);
			}
			break;
		case KeyEnd:
		case KeyEnd | Shift:
			if(true) {
				auto pb = getstart();
				auto ps = getcurrent();
				ps = linee(ps);
				set(ps - pb, (id & Shift) != 0);
			}
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
	const char*	geturl(stringbuilder& sb) const override {
		return url;
	}
public:
	bool open(const char* url) {
		auto s = 0;
		auto p = loadt(url, &s);
		if(!p)
			return false;
		this->url = szdup(url);
		reserve(s);
		setcount(s);
		memcpy(begin(), p, s);
		delete p;
		invalidate();
		return true;
	}
};

static struct editor_plugin : control::plugin, control::plugin::builder {
	editor_plugin() : plugin("editor", DockBottom) {}
	control* getcontrol() {
		return 0;
	}
	control::plugin::builder* getbuilder() {
		return this;
	}
	control* create(const char* url) override {
		auto p = new widget_editor();
		p->open(url);
		return p;
	}
	void destroy(control* p) override {
	}
	void getextensions(stringbuilder& sb) const override {
		sb.add("Исходный код (%1)", lex->extensions);
		sb.addsz();
		sb.add(lex->extensions);
		sb.addsz();
	}
} textview_plugin_instance;

void setlexer(lexer* v) {
	lex = v;
	auto old_font = draw::font;
	draw::font = fontedit;
	fontsize.x = textw('A');
	fontsize.y = texth();
	draw::font = old_font;
}