#include "crt.h"
#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

static control*	current_hilite;
static control*	current_focus;
static control::fncmd current_execute;
static control* current_execute_control;
const sprite* control::standart_toolbar = (sprite*)loadb("art/tools/toolbar.pma");
const sprite* control::standart_tree = (sprite*)loadb("art/tools/tree.pma");

const bsreq bsmeta<datetime>::meta[] = {{"datetime"}, {}};

bool control_input() {
	if(current_hilite) {
		switch(hot.key & CommandMask) {
		case MouseLeft:
		case MouseRight:
		case MouseLeftDBL:
			current_hilite->mouseinput(hot.key, hot.mouse);
			return true;
		case MouseWheelDown:
			current_hilite->mousewheel(hot.key, hot.mouse, 1);
			return true;
		case MouseWheelUp:
			current_hilite->mousewheel(hot.key, hot.mouse, -1);
			return true;
		}
	}
	if(current_focus) {
		if(current_focus->keyinput(hot.key))
			return true;
	}
	return false;
}

static struct control_plugin : draw::plugin {
	void before() override {
		current_hilite = 0;
		current_focus = 0;
		for(auto p = controls::control::plugin::first; p; p = p->next)
			p->before_render();
	}
} plugin_instance;

bool control::is(const char* s1, const char* s2) {
	if(!s1 || !s2)
		return false;
	return strcmp(s1, s2) == 0;
}

bool control::ishilited() const {
	return current_hilite == this;
}

bool control::isfocused() const {
	return draw::isfocused(*const_cast<control*>(this));
}

void control::setfocus(bool instant) {
	draw::setfocus(anyval(this, 0, 0), instant);
}

void control::mouseinput(unsigned id, point position) {
	switch(id) {
	case MouseLeft:
		if(hot.pressed)
			setfocus(true);
		break;
	case MouseRight:
		if(!hot.pressed) {
			redraw();
			updatewindow();
			contextmenu(getcommands());
		}
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
		if(strcmp(p->id, id) == 0)
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
	if(!this || !key)
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

void control::execute(control::fncmd proc, int param) const {
	current_execute = proc;
	current_execute_control = const_cast<control*>(this);
	draw::execute(control_execute, param);
}

void control::view(const rect& rc) {
	if(isfocusable()) {
		addelement(rc, *this);
		if(!draw::isfocused())
			setfocus(true);
	}
	if(isfocused())
		current_focus = this;
	if(areb(rc))
		current_hilite = this;
	if(show_background)
		rectf(rc, colors::window);
	if(show_border)
		rectb(rc, colors::border);
}

bool control::keyinput(unsigned key) {
	auto pn = getcommands()->find(key);
	if(!pn)
		return false;
	return (this->*pn->proc.cmd)(true);
}

void control::command::builder::render(const control* parent, const control::command* commands, bool& separator, int& count) {
	if(!commands)
		return;
	for(auto p = commands; *p; p++) {
		if(p->proc.visible && !(parent->*p->proc.visible)())
			continue;
		if(p->isgroup()) {
			separator = true;
			render(parent, p->child, separator, count);
			separator = true;
		} else {
			if(separator) {
				if(count>0)
					addseparator();
				separator = false;
			}
			add(parent, *p);
			count++;
		}
	}
}

bool control::command::isallow(const control* parent) const {
	if(!proc.cmd)
		return true;
	return (const_cast<control*>(parent)->*proc.cmd)(false);
}

void control::command::builder::render(const control* parent, const control::command* commands) {
	bool separator = false;
	auto count = 0;
	render(parent, commands, separator, count);
}

void control::contextmenu(const command* source, command::builder& pm) {
	if(!source)
		return;
	pm.start();
	pm.render(this, source);
	auto cmd = pm.finish();
	if(cmd)
		(this->*cmd->proc.cmd)(true);
}