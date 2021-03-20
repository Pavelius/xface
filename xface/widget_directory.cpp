#include "crt.h"
#include "datetime.h"
#include "draw_control.h"
#include "io.h"
#include "setting.h"
#include "stringbuilder.h"

using namespace	draw;
using namespace	draw::controls;

namespace {
struct header : tree::element {
	const char*		getname() const { return (const char*)object; }
};
}

static const char*		base_url = "D:/projects";

static struct widget_directory : tree, control::plugin {
	control* getcontrol() override {
		return this;
	}
	const char* getlabel(stringbuilder& sb) const override {
		return "Файлы";
	}
	void addurl(stringbuilder& sb, int index) {
		if(index < 0)
			return;
		addurl(sb, getparent(index));
		auto pb = (header*)get(index);
		if(sb)
			sb.add("/");
		sb.add(pb->getname());
	}
	void expanding(int index) override {
		char url[260]; stringbuilder sb(url);
		sb.add(base_url);
		if(index != -1)
			addurl(sb, index);
		for(io::file::find finder(url); finder; finder.next()) {
			auto pn = finder.name();
			if(pn[0] == '.')
				continue;
			auto ph = (header*)addnode(index, 1, 0, (void*)szdup(finder.name()));
			if(zchr(ph->getname(), '.') == 0) {
				ph->image = 1;
				ph->set(header::Group);
			}
		}
	}
	widget_directory() : tree(sizeof(header)), control::plugin("directory", DockLeft) {
		addstdimage();
		addcol("Наименование", ANREQ(header, object), "text").set(SizeAuto);
		no_change_count = true;
		read_only = true;
		select_mode = SelectRow;
		show_toolbar = false;
		show_header = false;
	}

} widget_control;

void directory_initialize() {
}

static setting::element tileset_url[] = {{"Директории", {base_url, setting::Url}},
};
static setting::header headers[] = {{"Рабочий стол", "Пути", 0, tileset_url},
};