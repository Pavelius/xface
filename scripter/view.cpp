#include "xface/crt.h"
#include "xface/draw_grid.h"
#include "xface/draw_properties.h"
#include "main.h"

using namespace code;
using namespace draw;

static auto code_font = (sprite*)loadb("art/fonts/code.pma");

bsreq metadata_type[] = {
	BSREQ(metadata, id, text_type),
	BSREQ(metadata, type, metadata_type),
	BSREQ(metadata, size, number_type),
{}};
BSMETA(metadata);
bsreq requisit_type[] = {
	BSREQ(requisit, id, text_type),
	BSREQ(requisit, type, metadata_type),
	BSREQ(requisit, parent, metadata_type),
	BSREQ(requisit, count, number_type),
	BSREQ(requisit, offset, number_type),
{}};
BSMETA(requisit);

struct translate {
	const char*			id;
	const char*			name;
	static int compare(const void* v1, const void* v2) {
		auto e1 = (translate*)v1;
		auto e2 = (translate*)v2;
		return strcmp(e1->id, e2->id);
	}
};
static translate translate_data[] = {{"count", "Количество"},
{"id", "Идентификатор"},
{"parent", "Родитель"},
{"type", "Тип"},
};

static struct metadata_control : controls::gridref, controls::control::plugin {
	void before_render() {
		if(metadata_instance.getcount()!=metadata_data.getcount()) {
			clear();
			for(auto& e : metadata_data) {
				if(!e)
					continue;
				add(&e);
			}
		}
		if(getcount() > 0)
			choose_metadata((struct metadata*)gridref::get(current));
	}
	control& getcontrol() override {
		return *this;
	}
	const char* getlabel(char* result, const char* result_maximum) const override {
		return "Типы";
	}
	command* getcommands() const override {
		return 0;
	}
	metadata_control() : gridref(metadata_type), plugin("metadata", DockLeft) {
		show_header = false;
		read_only = true;
		no_change_count = true;
		no_change_order = true;
		select_mode = SelectRow;
	}
} metadata_instance;

static struct requisit_control : controls::gridref, controls::control::plugin {
	
	struct metadata* current_parent;

	const char* getname(char* result, const char* result_max, struct metadata* type) const {
		if(type->ispointer()) {
			getname(result, result_max, type->dereference());
			szprint(zend(result), result_max, "*");
		} else
			szprint(result, result_max, type->id);
		return result;
	}

	const char* getname(char* result, const char* result_max, int line, int column) const override {
		if(strcmp(columns[column].id, "type")==0) {
			auto bv = getvalue(line, column);
			if(!bv)
				return "";
			result[0] = 0;
			auto rq = (requisit*)get(line);
			getname(result, result_max, (struct metadata*)bv.get());
			if(rq->count > 1)
				szprint(zend(result), result_max, "[%1i]", rq->count);
			return result;
		} else
			return gridref::getname(result, result_max, line, column);
	}

	control& getcontrol() override {
		return *this;
	}
	const char* getlabel(char* result, const char* result_maximum) const override {
		return "Реквизиты";
	}
	void before_render() {
		clear();
		if(current_parent) {
			for(auto& e : requisit_data) {
				if(e.parent == current_parent)
					add(&e);
			}
		}
	}
	command* getcommands() const override {
		return 0;
	}
	requisit_control() : gridref(requisit_type), plugin("requisit", DockLeftBottom) {
		show_header = false;
		read_only = true;
		no_change_count = true;
		no_change_order = true;
		select_mode = SelectRow;
	}
} requisit_instance;

static struct code_control : controls::scrollable, controls::control::plugin {
	control& getcontrol() override {
		return *this;
	}
	const char* getlabel(char* result, const char* result_maximum) const override {
		return "Скрипт";
	}
	code_control() : plugin("code", DockWorkspace) {
		maximum.y = 1000;
		maximum.x = 100;
	}

} code_instance;

void choose_metadata(metadata* v) {
	requisit_instance.current_parent = v;
}

static struct properties_control : controls::properties, controls::control::plugin {

	void view(const rect& rc) override {
		auto focus = (controls::control*)getfocus();
		bsval v;
		if(focus == &requisit_instance) {
			v.data = requisit_instance.getcurrent();
			v.type = requisit_type;
		} else if(focus == &metadata_instance) {
			v.data = metadata_instance.getcurrent();
			v.type = metadata_type;
		}
		if(v)
			set(v);
		properties::view(rc);
	}
	control& getcontrol() override {
		return *this;
	}
	const char* getlabel(char* result, const char* result_maximum) const override {
		return "Свойства";
	}
	const char* gettitle(char* result, const char* result_maximum, const bsval& ev) const override {
		translate k = {ev.type->id, 0};
		auto p = (translate*)bsearch(&k, translate_data, sizeof(translate_data) / sizeof(translate_data[0]), sizeof(translate_data[0]), translate::compare);
		if(p)
			return p->name;
		return ev.type->id;
	}
	bool isvisible(const bsval& ev) const {
		if(!ev)
			return false;
		if(strcmp(ev.type->id, "parent") == 0)
			return false;
		if(strcmp(ev.type->id, "offset") == 0)
			return false;
		if(strcmp(ev.type->id, "size") == 0)
			return false;
		return true;
	}
	properties_control() : plugin("properties", DockRight) { title = 100; }
} properties_instance;

void run_main() {
	requisit_instance.addcol("id", "Наименование", "text", SizeAuto);
	requisit_instance.addcol("type", "Тип", "ref", SizeFixed, 100);
	metadata_instance.addcol("id", "Наименование", "text", SizeAuto);
	draw::application("Scripter", false);
}