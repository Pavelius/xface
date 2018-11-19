#include "crt.h"
#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

static control*	current_hilite;
static control*	current_focus;
static control::callback current_execute;
static control* current_execute_control;
const sprite* control::standart_toolbar = (sprite*)loadb("art/tools/toolbar.pma");
const sprite* control::standart_tree = (sprite*)loadb("art/tools/tree.pma");

static struct control_plugin : draw::plugin {

	void before() override {
		current_hilite = 0;
		current_focus = 0;
	}

	bool translate(int id) override {
		if(current_hilite) {
			switch(id & CommandMask) {
			case MouseLeft:
			case MouseRight:
			case MouseLeftDBL:
				current_hilite->mouseinput(id, hot.mouse);
				return true;
			case MouseWheelDown:
				current_hilite->mousewheel(id, hot.mouse, 1);
				return true;
			case MouseWheelUp:
				current_hilite->mousewheel(id, hot.mouse, -1);
				return true;
			}
		}
		if(current_focus)
			return current_focus->keyinput(id);
		return false;
	}

} plugin_instance;

bool control::ishilited() const {
	return current_hilite == this;
}

bool control::isfocused() const {
	return current_focus == this;
}

void control::mouseinput(unsigned id, point position) {
	switch(id) {
	case MouseLeft:
		setfocus((int)this, true);
		break;
	}
}

static void control_execute() {
	(current_execute_control->*current_execute)(true);
}

const visual* visual::find(const char* id) const {
	if(!this)
		return 0;
	for(auto p = this; *p; p++) {
		if(p->id[0] == '*') {
			auto pf = p->child->find(id);
			if(pf)
				return pf;
		} else if(strcmp(p->id, id) == 0)
			return p;
	}
	return 0;
}

const control::command* control::command::find(const char* id) const {
	if(!this)
		return 0;
	for(auto p = this; *p; p++) {
		if(p->id[0] == '*') {
			auto p1 = p->child->find(id);
			if(p1)
				return p1;
		}
		if(strcmp(p->id, id) == 0)
			return p;
	}
	return 0;
}

const control::command* control::command::find(unsigned key) const {
	if(!this)
		return 0;
	for(auto p = this; *p; p++) {
		if(p->id[0] == '*') {
			auto p1 = p->child->find(key);
			if(p1)
				return p1;
		}
		if(p->key==key)
			return p;
	}
	return 0;
}

void control::icon(int x, int y, bool disabled, const command& cmd) const {
	image(x, y, getimages(), cmd.image, 0, disabled ? 0x80 : 0xFF);
}

void control::execute(control::callback proc) const {
	current_execute = proc;
	current_execute_control = const_cast<control*>(this);
	draw::execute(control_execute);
}

void control::view(const rect& rc) {
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

bool control::keyinput(unsigned key) {
	auto pn = getcommands()->find(key);
	if(!pn)
		return false;
	return (this->*pn->proc)(true);
}