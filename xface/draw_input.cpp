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
static bool			break_modal;
static int			break_result;
static userproc		input_proc;
plugin*				draw::plugin::first;
initplugin*			draw::initplugin::first;

static void set_focus_callback() {
	auto id = getnext(draw::getfocus(), hot.param);
	if(id)
		setfocus(id, true);
}

void draw::definput() {
	int id;
	switch(hot.key) {
	case KeyTab:
	case KeyTab | Shift:
	case KeyTab | Ctrl:
	case KeyTab | Ctrl | Shift:
		id = getnext(draw::getfocus(), hot.key);
		if(id)
			setfocus(id, true);
		break;
	case 0:
		exit(0);
		break;
	}
}

void draw::setinput(userproc proc) {
	input_proc = proc;
}

static void input_before() {
	hot.cursor = CursorArrow;
	render_control = elements;
	current_execute = 0;
	input_proc = definput;
	if(hot.mouse.x < 0 || hot.mouse.y < 0)
		sys_static_area.clear();
	else
		sys_static_area = {0, 0, draw::getwidth(), draw::getheight()};
	for(auto p = plugin::first; p; p = p->next)
		p->before();
}

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

void draw::execute(const hotinfo& value) {
	keep_hot = true;
	keep_hot_value = value;
	hot.key = InputUpdate;
}

plugin::plugin(int priority) : next(0), priority(priority) {
	if(!first)
		first = this;
	else {
		auto p = first;
		while(p->next && p->next->priority < priority)
			p = p->next;
		this->next = p->next;
		p->next = this;
	}
}

initplugin::initplugin(int priority) : next(0), priority(priority) {
	if(!first)
		first = this;
	else {
		auto p = first;
		while(p->next && p->next->priority < priority)
			p = p->next;
		this->next = p->next;
		p->next = this;
	}
}

void draw::breakmodal(int result) {
	break_modal = true;
	break_result = result;
}

void draw::buttoncancel() {
	breakmodal(0);
}

void draw::buttonok() {
	breakmodal(1);
}

int draw::getresult() {
	return break_result;
}

bool draw::ismodal() {
	input_before();
	if(!break_modal)
		return true;
	break_modal = false;
	return false;
}

void draw::initialize() {
	for(auto p = initplugin::first; p; p = p->next)
		p->initialize();
	for(auto p = initplugin::first; p; p = p->next)
		p->after_initialize();
	// Set default window colors
	draw::font = metrics::font;
	draw::fore = colors::text;
	draw::fore_stroke = colors::blue;
}

void draw::domodal() {
	if(current_execute) {
		auto proc = current_execute;
		input_before();
		proc();
		input_before();
		hot.key = InputUpdate;
		return;
	}
	for(auto p = plugin::first; p; p = p->next)
		p->after();
	if(hot.key == InputUpdate && keep_hot) {
		keep_hot = false;
		hot = keep_hot_value;
	} else
		hot.key = draw::rawinput();
	if(input_proc)
		input_proc();
}