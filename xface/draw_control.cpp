#include "crt.h"
#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

static control*			current_hilite;
static control*			current_focus;
static control::fncmd	current_execute;
const sprite*			control::standart_toolbar = (sprite*)loadb("art/tools/toolbar.pma");
const sprite*			control::standart_tree = (sprite*)loadb("art/tools/tree.pma");

bool control_input() {
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
	}
} plugin_instance;

bool control::equal(const char* s1, const char* s2) {
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

control* control::getfocus() {
	return current_focus;
}

void control::setfocus(bool instant) {
	draw::setfocus(anyval(this, 0, 0), instant);
}

static void control_execute() {
	(((control*)hot.object)->*current_execute)(true);
}
void control::postcmd(control::fncmd proc) const {
	current_execute = proc;
	draw::execute(control_execute, 0, 0, const_cast<control*>(this));
}

static void control_execute_keyinput() {
	((control*)hot.object)->keyinput(hot.param);
}
void control::postkeyinput(int value) const {
	draw::execute(control_execute_keyinput, value, 0, const_cast<control*>(this));
}

static void control_execute_setvalue() {
	((control*)hot.object)->setvalue((const char*)hot.param2, hot.param);
}
void control::postsetvalue(const char* id, int value) const {
	draw::execute(control_execute_setvalue, value, (int)id, const_cast<control*>(this));
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
		if(p->key == key)
			return p;
	}
	return 0;
}

void control::icon(int x, int y, bool disabled, const command& cmd) const {
	image(x, y, getimages(), cmd.image, 0, disabled ? 0x80 : 0xFF);
}

static void open_context_menu() {
	auto p = (control*)hot.param;
	p->redraw();
	updatewindow();
	p->contextmenu(p->getcommands());
}

void control::view(const rect& rc) {
	if(isfocusable()) {
		addelement(rc, *this);
		if(!draw::isfocused())
			setfocus(true);
	}
	if(isfocused())
		current_focus = this;
	if(ishilite(rc)) {
		current_hilite = this;
		switch(hot.key) {
		case MouseLeft:
			if(hot.pressed && !isfocused() && isfocusable())
				setfocus(false);
			break;
		case MouseRight:
			if(hot.pressed) {
				if(!isfocused() && isfocusable())
					setfocus(false);
			} else {
				if(isfocused())
					draw::execute(open_context_menu, (int)this);
			}
			break;
		}
	}
	if(show_background)
		rectf(rc, colors::window);
	if(show_border)
		rectb(rc, colors::border);
}

bool control::keyinput(unsigned key) {
	auto pn = getcommands()->find(key);
	if(!pn)
		return false;
	if(pn->proc.cmd_event) {
		pn->proc.cmd_event();
		return true;
	} else
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
				if(count > 0)
					addseparator();
				separator = false;
			}
			add(parent, *p);
			count++;
		}
	}
}

bool control::command::isallow(const control* parent) const {
	if(proc.cmd)
		return (const_cast<control*>(parent)->*proc.cmd)(false);
	return true;
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
	if(cmd) {
		if(cmd->proc.cmd)
			(this->*cmd->proc.cmd)(true);
		if(cmd->proc.cmd_event)
			cmd->proc.cmd_event();
	}
}

bool control::plugin::builder::canopen(const char* url) const {
	auto ext = szext(url);
	if(!ext)
		return false;
	ext--;
	char temp[1024 * 8]; stringbuilder sb(temp);
	getextensions(sb);
	sb.addsz();
	auto ps = zend(temp);
	while(ps[1]) {
		auto pe = ps + 1;
		if(szpmatch(ext, pe))
			return true;
		ps = zend(pe);
	}
	return false;
}

bool control::cut(bool run) {
	return false;
}

bool control::copy(bool run) {
	return false;
}

bool control::paste(bool run) {
	return false;
}

control::command control::commands_edit[] = {{"cut", "Вырезать", 0, &control::cut, 3, Ctrl + 'X'},
{"copy", "Копировать", 0, &control::copy, 4, Ctrl + 'C'},
{"paste", "Вставить", 0, &control::paste, 5, Ctrl + 'V'},
{}};