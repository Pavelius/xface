#include "crt.h"
#include "draw_control.h"

using namespace draw;

struct focusable_element {
	rect		rc;
	anyval		value;
};
static anyval	current_focus;
static anyval	cmd_value;
extern rect		sys_static_area;
static focusable_element elements[96];
static focusable_element* render_control;
static bool		break_modal;
static int		break_result;
plugin*			draw::plugin::first;
initplugin*		draw::initplugin::first;
fnevent			draw::domodal;

static void setfocus_callback() {
	setfocus(cmd_value, true);
}

static focusable_element* getby(const anyval& value) {
	if(!render_control)
		return 0;
	for(auto p = elements; p < render_control; p++) {
		if(p->value == value)
			return p;
	}
	return 0;
}

static focusable_element* getfirst() {
	if(!render_control)
		return 0;
	return elements;
}

static focusable_element* getlast() {
	if(!render_control || render_control == elements)
		return 0;
	return render_control - 1;
}

void draw::addelement(const rect& rc, const anyval& value) {
	if(!render_control)
		render_control = elements;
	else if(render_control >= elements + sizeof(elements) / sizeof(elements[0]))
		return;
	render_control->rc = rc;
	render_control->value = value;
	render_control++;
}

static const focusable_element* getnext(const focusable_element* pc, int key) {
	if(!key)
		return pc;
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
			return pe;
		switch(key) {
		case KeyRight:
			if(pe->rc.y1 >= pc->rc.y1
				&& pe->rc.y1 <= pc->rc.y2
				&& pe->rc.x1 < pc->rc.x1)
				return pc;
			break;
		case KeyLeft:
			if(pe->rc.y1 >= pc->rc.y1
				&& pe->rc.y1 <= pc->rc.y2
				&& pe->rc.x1 > pc->rc.x1)
				return pc;
			break;
		case KeyDown:
			if(pc->rc.y1 >= pe->rc.y2)
				return pc;
			break;
		case KeyUp:
			if(pc->rc.y2 <= pe->rc.y1)
				return pc;
			break;
		default:
			return pc;
		}
	}
}

bool draw::isfocused() {
	return static_cast<bool>(current_focus);
}

bool draw::isfocused(const anyval& v) {
	return current_focus == v;
}

bool draw::isfocused(const rect& rc, const anyval& value) {
	addelement(rc, value);
	if(!isfocused())
		setfocus(value, true);
	else if(ishilite(rc)) {
		switch(hot.key) {
		case MouseLeft:
		case MouseRight:
			if(hot.pressed)
				setfocus(value, false);
			break;
		}
	}
	return current_focus == value;
}

pushfocus::pushfocus() : value(const_cast<const anyval&>(current_focus)) {
	current_focus.clear();
}

pushfocus::~pushfocus() {
	current_focus = value;
}

void draw::setfocus(const anyval& value, bool instant) {
	if(current_focus==value)
		return;
	if(instant) {
		savefocus();
		current_focus = value;
	} else {
		cmd_value = value;
		auto old_hot = hot;
		execute(setfocus_callback);
		hot = old_hot;
	}
}

void draw::execute(fnevent proc, int param, int param2, void* object) {
	domodal = proc;
	hot.key = 0;
	hot.param = param;
	hot.param2 = param2;
	hot.object = object;
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

bool control_input();

static void standart_domodal() {
	for(auto p = plugin::first; p; p = p->next)
		p->after();
	hot.key = draw::rawinput();
	if(control_input())
		return;
	switch(hot.key) {
	case KeyTab:
	case KeyTab | Shift:
	case KeyTab | Ctrl:
	case KeyTab | Ctrl | Shift:
	case KeyUp:
	case KeyDown:
	case KeyRight:
	case KeyLeft:
		if(true) {
			auto pc = getnext(getby(current_focus), hot.key);
			if(pc)
				setfocus(pc->value, true);
		}
		break;
	case 0:
		exit(0);
		break;
	}
}

bool draw::ismodal() {
	hot.cursor = cursor::Arrow;
	render_control = elements;
	if(hot.mouse.x < 0 || hot.mouse.y < 0)
		sys_static_area.clear();
	else
		sys_static_area = {0, 0, draw::getwidth(), draw::getheight()};
	domodal = standart_domodal;
	for(auto p = plugin::first; p; p = p->next)
		p->before();
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
	draw::font = metrics::font;
	draw::fore = colors::text;
	draw::fore_stroke = colors::blue;
}