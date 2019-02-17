#include "xface/crt.h"
#include "xface/draw_grid.h"
#include "main.h"

using namespace draw;

void propset(const bsval& value);
void propset(bsval proc());
void propclear();

bsreq metadata_type[] = {
	BSREQ(metadata, id, text_type),
	BSREQ(metadata, type, metadata_type),
	BSREQ(metadata, size, number_type),
{}};
bsreq requisit_type[] = {
	BSREQ(requisit, id, text_type),
	BSREQ(requisit, type, metadata_type),
	BSREQ(requisit, parent, metadata_type),
	BSREQ(requisit, count, number_type),
	BSREQ(requisit, offset, number_type),
{}};

static struct metadata_control : controls::gridref, controls::control::plugin {
	void before_render() {
		if(metadata_instance.getcount()!=types.getcount()) {
			clear();
			for(auto& e : types) {
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
			for(auto& e : requisits) {
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

static struct code_control : controls::control, controls::control::plugin {
	control& getcontrol() override {
		return *this;
	}
	const char* getlabel(char* result, const char* result_maximum) const override {
		return "Скрипт";
	}
	code_control() : plugin("code", DockWorkspace) {}

} code_instance;

void choose_metadata(metadata* v) {
	requisit_instance.current_parent = v;
}

bsval detect_property() {
	auto focus = (controls::control*)getfocus();
	bsval v;
	if(focus == &requisit_instance) {
		v.data = requisit_instance.getcurrent();
		v.type = requisit_type;
	} else if(focus == &metadata_instance) {
		v.data = metadata_instance.getcurrent();
		v.type = metadata_type;
	}
	return v;
}

void run_main() {
	propclear();
	requisit_instance.addcol("id", "Наименование", "text", SizeAuto);
	requisit_instance.addcol("type", "Тип", "ref", SizeFixed, 100);
	metadata_instance.addcol("id", "Наименование", "text", SizeAuto);
	propset(detect_property);
	draw::application("Scripter", false);
}