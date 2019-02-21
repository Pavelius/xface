#include "xface/crt.h"
#include "xface/draw_control.h"
#include "main.h"

using namespace draw;
using namespace draw::controls;
using namespace code;

static auto code_font = (sprite*)loadb("art/fonts/code.pma");

static color code_colors[] = {{0, 0, 0}, {255, 255, 255}, {100, 100, 255},
{100, 100, 100}, {100, 100, 100},
{150, 150, 150}, {200, 150, 150}, {200, 200, 100}, {200, 200, 100},
};

namespace {
struct navigator : expression::builder {
	struct element {
		token_s				id;
		point				pos, size;
		const expression*	context;
		constexpr explicit operator bool() const { return id != DoNothing; }
		int					getcenter() const { return pos.x + size.x / 2; }
	};
	int						x, y;
	element*				p;
	element					elements[8192];
	navigator() : x(0), y(0), p(elements) {}
	void addline() override {
		x = 0;
		y++;
	}
	void add(token_s id, const expression* context, const char* v) override {
		if(id == OpenTag || id == CloseTag || id == Whitespace)
			return;
		p->id = id;
		p->pos.x = x;
		p->pos.y = y;
		p->size.y = 1;
		p->size.x = zlen(v);
		p->context = context;
		x += p->size.x;
		p++;
		if(p >= (elements + sizeof(elements) / sizeof(elements[0])))
			p = elements;
	}
	int getcount() const {
		return p - elements;
	}
	int find(const expression* v) const {
		for(auto p1 = elements; p1 < p; p1++) {
			if(p1->context == v)
				return p1 - elements;
		}
		return -1;
	}
	int getcolumn(int index) const {
		return elements[index].pos.x + elements[index].size.x / 2;
	}
	int getline(int index) const {
		return elements[index].pos.y;
	}
	point getposition(int index) const {
		auto pt = elements[index].pos;
		pt.x += elements[index].size.x / 2;
		return pt;
	}
	const element* getelement(int column, int line) const {
		const element* fe = 0;
		for(auto p1 = elements; p1 < p; p1++) {
			if(p1->pos.y != line)
				continue;
			if(column >= p1->pos.x && column <= (p1->pos.x + p1->size.x))
				fe = p1;
			else {
				auto d1 = iabs(line - p1->getcenter());
				if(!fe || d1 < iabs(line - fe->getcenter()))
					fe = p1;
			}
		}
		return fe;
	}
	const expression* getcontext(int column, int line) const {
		auto fe = getelement(column, line);
		if(fe)
			return fe->context;
		return 0;
	}
};
struct renderer : expression::builder {
	const rect&			rc;
	int					x, y;
	const expression*	hilite;
	rect				hilite_rect;
	const expression*	current;
	rect				current_rect;
	constexpr renderer(const rect& rc) : rc(rc),
		x(rc.x1 + metrics::padding),
		y(rc.y1 + metrics::padding),
		hilite(), hilite_rect(),
		current(0), current_rect() {}
	void addline() override {
		x = rc.x1 + metrics::padding;
		y = y + texth();
	}
	void add(token_s id, const expression* context, const char* v) override {
		auto selected = (current == context && id != Whitespace);
		auto push_fore = fore;
		auto w = textw(v);
		rect rc = {x, y, x + w, y + texth()};
		auto a = area(rc);
		if(a == AreaHilited || a == AreaHilitedPressed) {
			hilite = context;
			hilite_rect = rc;
		}
		if(selected) {
			rectf(rc, colors::edit);
			current_rect = rc;
		}
		fore = code_colors[id];
		text(x, y, v);
		x += w;
		fore = push_fore;
	}
};
}

static struct code_control : controls::control, controls::control::plugin {
	requisit*			source;
	const expression*	current;
	rect				current_rect;
	const expression*	current_hilite;
	rect				current_hilite_rect;

	control& getcontrol() override {
		return *this;
	}
	const char* getlabel(char* result, const char* result_maximum) const override {
		return "Ñêðèïò";
	}
	static void select_mouse() {
		auto p = (code_control*)hot.param;
		p->current = p->current_hilite;
		p->current_rect = p->current_hilite_rect;
	}
	const expression* navigate_key(const expression* current, unsigned key) {
		if(!source)
			return 0;
		navigator b;
		source->code->add(b);
		auto index = b.find(current);
		if(index == -1)
			return 0;
		point pt;
		auto index_maximum = b.getcount();
		switch(key) {
		case KeyLeft:
			if(index > 0)
				return b.elements[index - 1].context;
			break;
		case KeyRight:
			if(index < index_maximum - 1)
				return b.elements[index + 1].context;
			break;
		case Ctrl | KeyHome:
			if(index_maximum > 0)
				return b.elements[0].context;
			break;
		case KeyHome:
			while(index > 0 && b.elements[index].pos.y == b.elements[index - 1].pos.y)
				index--;
			return b.elements[index].context;
		case KeyEnd:
			while(index < index_maximum - 1 && b.elements[index].pos.y == b.elements[index + 1].pos.y)
				index++;
			return b.elements[index].context;
		case Ctrl | KeyEnd:
			if(index_maximum > 0)
				return b.elements[index_maximum - 1].context;
			break;
		case KeyUp:
			pt = b.getposition(index);
			return b.getcontext(pt.x, pt.y - 1);
		case KeyDown:
			pt = b.getposition(index);
			return b.getcontext(pt.x, pt.y + 1);
		default:
			break;
		}
		return 0;
	}
	code_control() : plugin("code", DockWorkspace), source(0) {}
	void view(const rect& rc) {
		control::view(rc);
		auto push_font = font;
		font = code_font;
		current_hilite = 0;
		if(source) {
			renderer b(rc);
			b.current = current;
			source->code->add(b);
			current_hilite = b.hilite;
			current_hilite_rect = b.hilite_rect;
			current_rect = b.current_rect;
		}
		if(hot.key == MouseLeft && hot.pressed && current_hilite)
			draw::execute(select_mouse, (int)this);
		font = push_font;
	}
	void dropdown(char start_symbol = 0) {
		if(current) {
			char filter[] = {start_symbol, 0};
			valuelist e;
			current->select(e);
			auto w = current_rect.width();
			if(w < 100)
				w = 100;
			choose(current_rect.x1, current_rect.y2, w, e, filter, control::standart_tree);
		}
	}
	void addnothing(expression* p) {
		if(!p)
			return;
		if(p->type == DoNothing)
			return;
		if(p->getoperands() != Statement)
			return;
		if(p->next) {
			if(p->next->getoperands() != Statement)
				return;
			if(p->next->type == DoNothing)
				return;
		}
		p->add(new expression(DoNothing));
	}
	bool keyinput(unsigned id) {
		const expression* pe;
		switch(id&0xFFFF) {
		case KeyLeft:
		case KeyRight:
		case KeyDown:
		case KeyUp:
		case KeyHome:
		case KeyEnd:
			pe = navigate_key(current, id);
			if(pe)
				current = pe;
			break;
		case Alpha + '0':
		case Alpha + '1':
		case Alpha + '2':
		case Alpha + '3':
		case Alpha + '4':
		case Alpha + '5':
		case Alpha + '6':
		case Alpha + '7':
		case Alpha + '8':
		case Alpha + '9':
			break;
		case KeyDelete:
			if(current)
				const_cast<expression*>(current)->zero();
			break;
		case KeySpace:
			dropdown();
			break;
		case KeyEnter:
			addnothing(const_cast<expression*>(current));
			break;
		case InputSymbol:
			if(hot.param > 32)
				dropdown(hot.param);
			break;
		default:
			return control::keyinput(id);
		}
		return true;
	}
} code_instance;

void setcode(requisit* v) {
	code_instance.source = v;
}