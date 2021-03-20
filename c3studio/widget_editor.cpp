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

class widget_editor : public control, vector<char> {
	int					cash_origin = -1;
	int					lines_per_page = 0;
	int					p1 = 0, p2 = 0;
	pointl				pos1 = {}, pos2 = {}, size = {};
	pointl				origin = {}, maximum = {};
	rect				rctext = {4, 4, 4, 4};
	bool				readonly = false;
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
	int	linen(int index) const {
		return 0;
	}
	int	hittest(rect rc, point pt, unsigned state) const {
		return draw::hittest({rc.x1, rc.y1 - origin.y, rc.x2, rc.y2}, ptr(0), state, pt);
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
			point mpos;
			mpos.x = (hot.mouse.x - r1.x1 + origin.x + fontsize.x / 2) / fontsize.x;
			mpos.y = (hot.mouse.y - r1.y1) / fontsize.y + origin.y;
			auto x1 = rc.x1 + mpos.x * fontsize.x;
			auto y1 = rc.y1 + mpos.y * fontsize.y;
			rect r2 = {x1, y1, x1 + fontsize.x, y1 + fontsize.y};
			ishilite(r2);
		}
		pointl pos = {};
		group_s type;
		auto ps = begin();
		while(true) {
			auto x1 = x + pos.x * fontsize.x;
			auto y1 = y + pos.y * fontsize.y;
			auto c = lex->getnext(ps, pos, type);
			if(!c)
				break;
			auto& ei = bsdata<groupi>::elements[type];
			fore = ei.present;
			text(x1, y1, ps, c, ei.flags);
			ps += c;
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
	bool open(const char* url) {
		auto s = 0;
		auto p = loadt(url, &s);
		if(!p)
			return false;
		reserve(s);
		memcpy(begin(), p, s);
		delete p;
		invalidate();
		return true;
	}
public:
	widget_editor(const char* url) {
		open(url);
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
		return new widget_editor(url);
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