#include "crt.h"
#include "datetime.h"
#include "draw_control.h"
#include "io.h"
#include "settings.h"
#include "stringbuilder.h"

using namespace	draw;
using namespace	draw::controls;

namespace {
struct header : tableref::element {
	const char*		name;
	const char* getname() const { return name; }
	header* getparent() const { return (header*)object; }
	void geturl(stringbuilder& sb) const {
		if(getparent()) {
			getparent()->geturl(sb);
			sb.add("/");
		}
		sb.add(getname());
	}
};
}
INSTMETA(header) = {BSREQ(name), BSREQ(image), {}};

static struct widget_directory : control::plugin, tableref {

	const char* base_url;

	void after_initialize() override {
		auto meta = bsmeta<header>::meta;
		addstdimage();
		addcol(meta, "name", "Наименование").set(SizeAuto);
		expand(0, 0);
	}

	control& getcontrol() override {
		return *this;
	}

	const char* getlabel(char* result, const char* result_maximum) const override {
		return "Файлы";
	}

	void expanding(builder& e) override {
		auto p = base_url;
		for(io::file::find finder(p); finder; finder.next()) {
			auto ph = (header*)e.add(0, 1, 0, true);
			ph->name = szdup(finder.name());
			ph->type = 1;
		}
	}

	widget_directory() : control::plugin("directory", DockLeft) {
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