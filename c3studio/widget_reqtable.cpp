#include "draw_control.h"
#include "package.h"

using namespace code;
using namespace draw;

BSDATAC(package, 2048)

static package* add_package() {
	for(auto& e : bsdata<package>()) {
		if(!e)
			return &e;
	}
	return bsdata<package>::add();
}

static const char* getpackagename(const void* object, stringbuilder& sb) {
	auto p = (package*)object;
	return p->getsymurl(0);
}

class view_packages : public controls::table {
	void* addrow() override {
		return add_package();
	}
	int getmaximum() const override {
		return bsdata<package>::source.getcount();
	}
	void* get(int index) const override {
		return bsdata<package>::source.ptr(index);
	}
	void remove(int index) override {
		auto p = bsdata<package>::elements + index;
		p->clear();
		bsdata<package>::source.remove(index);
	}
	void swap(int i1, int i2) override {
		bsdata<package>::source.swap(i1, i2);
	}
	const char* geturl(stringbuilder& sb) const override {
		return sb;
	}
public:
	view_packages() {
		auto& c0 = addcol("Изображение", "image");
		auto& c1 = addcol("Имя", "text");
		c1.plist.getname = getpackagename;
		c1.flags.add(ColumnReadOnly);
		select_mode = SelectRow;
	}
};

class widget_packages : view_packages, draw::controls::control::plugin {
	control* getcontrol() override {
		return this;
	}
public:
	widget_packages() : plugin("code", DockLeft) {
		show_header = false;
		show_toolbar = false;
	}
};
static widget_packages instance;

void add_package(const char* id) {
	char temp[260]; stringbuilder sb(temp);
	if(!package::findurl(sb, id, "ast"))
		return;
	auto p = add_package();
	if(!p)
		return;
	p->addsym(p->addstr("this"), Class, p->addstr(id), 0, 0, 0);
	p->read(temp);
}