#include "xface/crt.h"
#include "xface/draw_grid.h"
#include "xface/draw_properties.h"
#include "main.h"

using namespace code;
using namespace draw;

const bsreq bsmeta<metadata>::meta[] = {
	BSREQ(id),
	BSREQ(type),
	BSREQ(size),
{}};
const bsreq bsmeta<requisit>::meta[] = {
	BSREQ(id),
	BSREQ(type),
	BSREQ(count),
	BSREQ(offset),
	BSREQ(position),
{}};

static controls::properties::translate translate_data[] = {{"count", "Количество"},
{"id", "Идентификатор"},
{"parent", "Родитель"},
{"position", "Позиция"},
{"type", "Тип"},
};

static struct metadata_control : controls::gridref, controls::control::plugin {
	void before_render() {
		if(metadata_instance.getcount() != config.types.getcount()) {
			clear();
			//for(auto& e : config.types) {
			//	if(!e)
			//		continue;
			//	if(e.isreference() || e.isarray())
			//		continue;
			//	add(&e);
			//}
		}
		if(getcount() > 0)
			((metadata*)gridref::get(current))->hilite();
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
	metadata_control() : gridref(bsmeta<metadata>::meta), plugin("metadata", DockLeft) {
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
		if(type->isreference()) {
			getname(result, result_max, type->type);
			szprint(zend(result), result_max, "*");
		} else if(type->isarray()) {
			getname(result, result_max, type->type);
			szprint(zend(result), result_max, "[]");
		} else
			szprint(result, result_max, type->id);
		return result;
	}

	const char* getname(char* result, const char* result_max, int line, int column) const override {
		//if(strcmp(columns[column].id, "type") == 0) {
		//	auto bv = getvalue(line, column);
		//	if(!bv)
		//		return "";
		//	result[0] = 0;
		//	auto rq = (requisit*)get(line);
		//	getname(result, result_max, (struct metadata*)bv.get());
		//	if(rq->count > 1)
		//		szprint(zend(result), result_max, "[%1i]", rq->count);
		//	return result;
		//} else
		//	return gridref::getname(result, result_max, line, column);
		return "";
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
			for(auto& e : current_parent->requisits)
				add(&e);
		}
		//if((unsigned)current < getcount())
		//	setcode(((requisit**)data)[current]);
	}
	command* getcommands() const override {
		return 0;
	}
	requisit_control() : gridref(bsmeta<requisit>::meta), plugin("requisit", DockLeftBottom) {
		show_header = false;
		read_only = true;
		no_change_count = true;
		no_change_order = true;
		select_mode = SelectRow;
	}
} requisit_instance;

void metadata::hilite() {
	requisit_instance.current_parent = this;
}

static struct properties_control : controls::properties, controls::control::plugin {

	void view(const rect& rc) override {
		auto focus = (controls::control*)isfocused();
		bsval v;
		if(focus == &requisit_instance) {
			v.data = requisit_instance.getcurrent();
			v.type = bsmeta<requisit>::meta;
		} else if(focus == &metadata_instance) {
			v.data = metadata_instance.getcurrent();
			v.type = bsmeta<metadata>::meta;
		}
		if(v)
			set(v);
		properties::view(rc);
	}
	static void choose_metadata(adat<void*, 64>& result, const bsreq** name_requisit, void* type) {
		if(name_requisit)
			*name_requisit = bsmeta<metadata>::meta;
		//for(auto& e : config.types) {
		//	if(!e)
		//		continue;
		//	if(e.isreference() || e.isarray())
		//		continue;
		//	result.add(&e);
		//}
		for(auto p : config.standart)
			result.add(p);
	}
	//listproc* getprocchoose(const bsreq* type) const {
	//	if(type->type == bsmeta<metadata>::meta)
	//		return choose_metadata;
	//	return 0;
	//}
	control& getcontrol() override {
		return *this;
	}
	const char* getlabel(char* result, const char* result_maximum) const override {
		return "Свойства";
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
	properties_control() : plugin("properties", DockRight) {
		title = 100;
		for(auto& e : translate_data)
			dictionary.add(e);
	}
} properties_instance;

void run_main() {
	//requisit_instance.addcol("id", "Наименование", "text", SizeAuto);
	//requisit_instance.addcol("type", "Тип", "ref", SizeFixed, 100);
	//metadata_instance.addcol("id", "Наименование", "text", SizeAuto);
	draw::application("Scripter", false);
}