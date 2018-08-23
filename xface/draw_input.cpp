#include "crt.h"
#include "draw.h"

struct focusable_element {
	int				id;
	rect			rc;
	operator bool() const { return id != 0; }
};
static int			current_command;
static int			current_focus;
static void			(*current_execute)();
extern rect			sys_static_area;
static focusable_element	elements[96];
static focusable_element*	render_control;

static struct input_plugin : draw::renderplugin {

	void before() override {
		render_control = elements;
		current_command = 0;
		current_execute = 0;
		hot::cursor = CursorArrow;
		if(hot::mouse.x < 0 || hot::mouse.y < 0)
			sys_static_area.clear();
		else
			sys_static_area = {0, 0, draw::getwidth(), draw::getheight()};
	}

	bool translate(int id) override {
		int focus;
		switch(id) {
		case KeyTab:
			focus = draw::getfocus();
			focus = draw::getnext(focus, KeyTab);
			draw::setfocus(focus, true);
			return true;
		}
		return false;
	}

} input_plugin_instance;

static void setfocus_callback() {
	current_focus = hot::param;
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
	else if(current_focus!=id)
		execute(setfocus_callback, id);
}

int draw::getfocus() {
	return current_focus;
}

void draw::execute(int id, int param) {
	current_command = id;
	hot::key = 0;
	hot::param = param;
}

void draw::execute(void(*proc)(), int param) {
	execute(InputExecute, param);
	current_execute = proc;
}

int draw::input(bool redraw) {
	if(current_command) {
		if(current_execute) {
			current_execute();
			hot::key = InputUpdate;
			return hot::key;
		}
		hot::key = current_command;
		return hot::key;
	}
	// After render plugin events
	for(auto p = renderplugin::first; p; p = p->next)
		p->after();
	hot::key = InputUpdate;
	if(redraw)
		draw::sysredraw();
	else
		hot::key = draw::rawinput();
	if(!hot::key)
		exit(0);
	return hot::key;
}