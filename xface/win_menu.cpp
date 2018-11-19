#include "win.h"
#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

struct menu_builder : control::command::builder {

	control*		parent;
	void*			hMenu;
	
	menu_builder(control* parent) : hMenu(0), parent(parent) {
		hMenu = CreatePopupMenu();
	}

	~menu_builder() {
		DestroyMenu(hMenu);
	}

	void add(const control::command& cmd) override {
	}

	void addseparator() override {
	}

};

control::command::builder* control::createmenu() {
	return new menu_builder(this);
}