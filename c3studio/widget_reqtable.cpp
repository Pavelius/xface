#include "draw_control.h"
#include "package.h"

using namespace code;
using namespace draw;

BSDATAC(package, 1024)

static package* add_package() {
	for(auto& e : bsdata<package>()) {
		if(!e)
			return &e;
	}
	return bsdata<package>::add();
}

class packages_table : public controls::table {
	void* addrow() override {
		return add_package();
	}
	int getmaximum() const override {
		return bsdata<package>::source.getcount();
	}
	int getimage(int row) const override {
		return 2;
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
	static const char* getname(const void* object, stringbuilder& sb) {
		auto p = (package*)object;
		return p->getsymurl(0);
	}
public:
	packages_table() {
		auto& c0 = addcol("Изображение", "standart_image");
		auto& c1 = addcol("Имя", "text");
		c1.plist.getname = getname;
		c1.flags.add(ColumnReadOnly);
		select_mode = SelectRow;
	}
};

class packages_tree : public controls::tree {
	enum type_s {
		Package, Symbol,
	};
	package* getpackage(int index) const {
		while(index!=-1) {
			auto p = (element*)get(index);
			if(p->type == Package)
				return (package*)p->object;
			index = getparent(index);
		}
		return 0;
	}
	void expanding(int index) override {
		package* pk;
		pckh ph;
		if(index == -1) {
			for(auto& e : bsdata<package>()) {
				if(!e)
					continue;
				addnode(index, Package, 5, &e);
			}
		} else {
			auto p = (element*)get(index);
			switch(p->type) {
			case Symbol:
				pk = getpackage(index);
				if(!pk)
					break;
				ph = (symbol*)p->object - pk->symbols.begin();
				for(auto& e : pk->symbols) {
					if(e.parent != ph)
						continue;
					addnode(index, Symbol, 0, &e, false);
				}
				break;
			case Package:
				pk = getpackage(index);
				if(!pk)
					break;
				for(auto& e : pk->symbols) {
					if(e.parent != This)
						continue;
					addnode(index, Symbol, 5, &e);
				}
				break;
			}
		}
	}
	static const char* getname(const void* object, stringbuilder& sb) {
		auto p = (element*)object;
		package* pk;
		symbol* ps;
		switch(p->type) {
		case Symbol:
			ps = (symbol*)p->object;
			return "Symbol";
		case Package:
			pk = (package*)p->object;
			return pk->getstr(0);
		default:
			return "None";
		}
	}
public:
	packages_tree() {
		auto& c0 = addcol("Изображение", "standart_image");
		auto& c1 = addcol("Имя", "text");
		c1.plist.getname = getname;
		c1.set(ColumnReadOnly);
		c1.set(SizeAuto);
		select_mode = SelectText;
	}
};

class widget_packages : public packages_tree, draw::controls::control::plugin {
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