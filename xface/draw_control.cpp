#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

static control*	current_hilite;
static control*	current_focus;
static void (control::*current_execute)();
static control* current_execute_control;

static struct control_plugin : draw::plugin {

	void before() override {
		current_hilite = 0;
		current_focus = 0;
		current_execute = 0;
		current_execute_control = 0;
	}

	bool translate(int id) override {
		if(current_focus) {
			switch(id&0xFFFF) {
			case KeyBackspace: current_focus->keybackspace(id); return true;
			case KeyDelete: current_focus->keydelete(id); return true;
			case KeyEnter: current_focus->keyenter(id); return true;
			case KeyUp: current_focus->keyup(id); return true;
			case KeyDown: current_focus->keydown(id); return true;
			case KeyLeft: current_focus->keyleft(id); return true;
			case KeyRight: current_focus->keyright(id); return true;
			case KeyHome: current_focus->keyhome(id); return true;
			case KeyEnd: current_focus->keyend(id); return true;
			case KeyPageUp: current_focus->keypageup(id); return true;
			case KeyPageDown: current_focus->keypagedown(id); return true;
			case InputSymbol: current_focus->keysymbol(hot.param); break;
			}
		}
		if(current_hilite) {
			switch(id) {
			case MouseLeft: current_hilite->mouseleft(hot.mouse); return true;
			case MouseLeftDBL: current_hilite->mouseleftdbl(hot.mouse); return true;
			case MouseWheelUp: current_hilite->mousewheel(hot.mouse, -1); return true;
			case MouseWheelDown: current_hilite->mousewheel(hot.mouse, 1); return true;
			}
		}
		return false;
	}

} plugin_instance;

control::control() : show_border(true), show_background(true) {
}

bool control::ishilited() const {
	return current_hilite == this;
}

bool control::isfocused() const {
	return current_focus == this;
}

void control::mouseleft(point position) {
	setfocus((int)this, true);
}

static void control_execute() {
	(current_execute_control->*current_execute)();
}

void control::execute(void (control::*proc)()) const {
	current_execute = proc;
	current_execute_control = (control*)this;
	draw::execute(control_execute);
}

void control::view(rect rc) {
	if(isfocusable()) {
		addelement((int)this, rc);
		if(!getfocus())
			setfocus((int)this, true);
	}
	if(areb(rc))
		current_hilite = this;
	if((control*)getfocus() == this)
		current_focus = this;
	if(show_background)
		rectf(rc, colors::window);
	if(show_border)
		rectb(rc, colors::border);
}