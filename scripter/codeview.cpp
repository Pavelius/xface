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
	void begin() override {
	}
	void end() override {
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
	bool				focused;
	const expression*	hilite;
	rect				hilite_rect;
	const expression*	current;
	rect				current_rect;
	int					level;
	constexpr renderer(const rect& rc, bool focused) : rc(rc), focused(focused),
		x(rc.x1 + metrics::padding),
		y(rc.y1 + metrics::padding),
		hilite(), hilite_rect(),
		current(0), current_rect(), level(0) {}
	void addline() override {
		x = rc.x1 + metrics::padding + level*textw("A");
		y = y + texth();
	}
	void begin() override {
		level++;
	}
	void end() override {
		level--;
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
			if(focused)
				rectf(rc, colors::edit);
			else
				rectf(rc, colors::edit.mix(colors::window));
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
	const expression* get_statement(const expression* current) {
		return navigate_key(current, KeyHome);
	}
	code_control() : plugin("code", DockWorkspace), source(0) {}
	void view(const rect& rc) {
		control::view(rc);
		auto push_font = font;
		font = code_font;
		current_hilite = 0;
		if(source) {
			renderer b(rc, isfocused());
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
			auto result = const_cast<expression*>(current);
			char filter[] = {start_symbol, 0};
			valuelist e;
			current->select(e);
			auto w = current_rect.width();
			if(w < 100)
				w = 100;
			auto pe = choose(current_rect.x1, current_rect.y2, w, e, filter, control::standart_tree);
			if(pe) {
				result->set((expression_s)pe->type);
				switch(result->type) {
				case Requisit:
					result->req = (requisit*)pe->value;
					break;
				case Metadata:
					result->met = (struct metadata*)pe->value;
					break;
				}
			}
		}
	}
	void editstring(const expression* p) {
		if(!p)
			return;
		if(p->type != Text)
			return;
		char temp[256*4*8];
		textedit te(temp, sizeof(temp), true);
		auto w = current_rect.width();
		if(w < 100)
			w = 100;
		zcpy(temp, p->text, sizeof(temp));
		rect rc = {current_rect.x1, current_rect.y1, current_rect.x1 + w, current_rect.y1 + texth() * 2+ 8};
		te.editing(rc);
	}
	const expression* addnothing(expression* p) {
		if(!p)
			return 0;
		if(p->type == DoNothing)
			return 0;
		if(p->getoperands() != Statement)
			return 0;
		if(p->next) {
			if(p->next->getoperands() != Statement)
				return 0;
			if(p->next->type == DoNothing)
				return 0;
		}
		auto result = new expression(DoNothing);
		p->add(result);
		return result;
	}
	void select(const expression* pe) {
		if(!pe)
			return;
		current = pe;
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
			select(navigate_key(current, id));
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
			pe = addnothing(const_cast<expression*>(get_statement(current)));
			select(pe);
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
	if(!code_instance.current && v)
		code_instance.current = v->code;
}