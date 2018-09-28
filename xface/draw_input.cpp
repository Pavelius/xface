#include "crt.h"
#include "draw.h"

using namespace draw;

struct focusable_element {
	int				id;
	rect			rc;
	operator bool() const { return id != 0; }
};
static int			current_focus;
static void(*current_execute)();
extern rect			sys_static_area;
static bool			keep_hot;
static hotinfo		keep_hot_value;
static focusable_element	elements[96];
static focusable_element*	render_control;

static void set_focus_callback() {
	auto id = getnext(draw::getfocus(), hot.param);
	if(id)
		setfocus(id, true);
}

static struct input_plugin : draw::plugin {

	void before() override {
		hot.cursor = CursorArrow;
		render_control = elements;
		current_execute = 0;
		if(hot.mouse.x < 0 || hot.mouse.y < 0)
			sys_static_area.clear();
		else
			sys_static_area = {0, 0, draw::getwidth(), draw::getheight()};
	}

	bool translate(int id) override {
		switch(id) {
		case KeyTab:
		case KeyTab | Shift:
		case KeyTab | Ctrl:
		case KeyTab | Ctrl | Shift:
			id = getnext(draw::getfocus(), id);
			if(id)
				setfocus(id, true);
			return true;
		case 0:
			exit(0);
			return true;
		}
		return false;
	}

} plugin_instance;

static void setfocus_callback() {
	current_focus = hot.param;
}

static focusable_element* getby(int id) {
	if(!id)
		return 0;
	for(auto& e : elements) {
		if(!e)
			return 0;
		if(e.id == id)
			return &e;
	}
	return 0;
}

static focusable_element* getfirst() {
	for(auto& e : elements) {
		if(!e)
			return 0;
		return &e;
	}
	return 0;
}

static focusable_element* getlast() {
	auto p = elements;
	for(auto& e : elements) {
		if(!e)
			break;
		p = &e;
	}
	return p;
}

void draw::addelement(int id, const rect& rc) {
	if(!render_control
		|| render_control >= elements + sizeof(elements) / sizeof(elements[0]) - 1)
		render_control = elements;
	render_control[0].id = id;
	render_control[0].rc = rc;
	render_control[1].id = 0;
	render_control++;
}

int draw::getnext(int id, int key) {
	if(!key)
		return id;
	auto pc = getby(id);
	if(!pc)
		pc = getfirst();
	if(!pc)
		return 0;
	auto pe = pc;
	auto pl = getlast();
	int inc = 1;
	if(key == KeyLeft || key == KeyUp || key == (KeyTab | Shift))
		inc = -1;
	while(true) {
		pc += inc;
		if(pc > pl)
			pc = elements;
		else if(pc < elements)
			pc = pl;
		if(pe == pc)
			return pe->id;
		switch(key) {
		case KeyRight:
			if(pe->rc.y1 >= pc->rc.y1
				&& pe->rc.y1 <= pc->rc.y2
				&& pe->rc.x1 < pc->rc.x1)
				return pc->id;
			break;
		case KeyLeft:
			if(pe->rc.y1 >= pc->rc.y1
				&& pe->rc.y1 <= pc->rc.y2
				&& pe->rc.x1 > pc->rc.x1)
				return pc->id;
			break;
		case KeyDown:
			if(pc->rc.y1 >= pe->rc.y2)
				return pc->id;
			break;
		case KeyUp:
			if(pc->rc.y2 <= pe->rc.y1)
				return pc->id;
			break;
		default:
			return pc->id;
		}
	}
}

void draw::setfocus(int id, bool instant) {
	if(instant)
		current_focus = id;
	else if(current_focus != id)
		execute(setfocus_callback, id);
}

int draw::getfocus() {
	return current_focus;
}

void draw::execute(void(*proc)(), int param) {
	current_execute = proc;
	hot.key = 0;
	hot.param = param;
}

//void draw::execute(const hotinfo& value) {
//	keep_hot = true;
//	keep_hot_value = value;
//	hot.key = InputUpdate;
//}

void draw::domodal() {
	if(hot.key == InputUpdate) {
		if(keep_hot) {
			keep_hot = false;
			hot = keep_hot_value;
			return;
		}
	}
	if(current_execute) {
		auto proc = current_execute;
		for(auto p = plugin::first; p; p = p->next)
			p->before();
		proc();
		if(keep_hot)
			return;
		for(auto p = plugin::first; p; p = p->next)
			p->before();
		hot.key = InputUpdate;
		return;
	}
	for(auto p = plugin::first; p; p = p->next)
		p->after();
	hot.key = draw::rawinput();
	for(auto p = plugin::first; p; p = p->next) {
		if(p->translate(hot.key))
			break;
	}
}