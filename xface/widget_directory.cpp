#include "crt.h"
#include "datetime.h"
#include "draw_control.h"
#include "io.h"
#include "settings.h"
#include "stringbuilder.h"

using namespace	draw;
using namespace	draw::controls;

namespace {
struct header : tree::element {
	const char*		name;
	const char* getname() const { return name; }
};
}
INSTMETA(header) = {BSREQ(name), BSREQ(image), {}};

static struct widget_directory : tree, control::plugin {

	const char* base_url;

	void after_initialize() override {
		auto meta = bsmeta<header>::meta;
		addstdimage();
		addcol(meta, "name", "Наименование").set(SizeAuto);
		expand(0);
	}

	control& getcontrol() override {
		return *this;
	}

	const char* getlabel(char* result, const char* result_maximum) const override {
		return "Файлы";
	}

	void addurl(stringbuilder& sb, int index) {
		if(index < 0)
			return;
		addurl(sb, getparent(index));
		auto pb = (header*)get(index);
		if(sb)
			sb.add("/");
		sb.add(pb->name);
	}

	void expanding(int index, int level) override {
		char url[260]; stringbuilder sb(url);
		sb.add(base_url);
		if(level > 1)
			addurl(sb, index-1);
		for(io::file::find finder(url); finder; finder.next()) {
			auto pn = finder.name();
			if(pn[0] == '.')
				continue;
			auto ph = (header*)insert(index, level);
			ph->name = szdup(finder.name());
			if(zchr(ph->name, '.') == 0) {
				ph->image = 1;
				ph->setgroup(true);
			}
		}
	}

	widget_directory() : tree(sizeof(header)), control::plugin("directory", DockLeft) {
		no_change_count = true;
		read_only = true;
		select_mode = SelectRow;
		show_toolbar = false;
		show_header = false;
		base_url = "D:/projects";
	}

} widget_control;

void directory_initialize() {
}