#include "xface/crt.h"
#include "xface/draw_properties.h"
#include "main.h"

using namespace code;
using namespace draw;

static controls::properties::translate translate_data[] = {{"count", "Количество"},
{"id", "Идентификатор"},
{"parent", "Родитель"},
{"type", "Тип"},
};

static class metadata_control : public controls::tableref, controls::control::plugin {
	void after_initialize() override {
		auto meta = bsmeta<metadata>::meta;
		addstdimage();
		addcol(meta, "id", "Наименование").set(SizeAuto);
		update();
	}
	void update() {
		clear();
		for(auto& e : bsdata<metadata>()) {
			if(!e)
				continue;
			if(e.isreference() || e.isarray() || e.ispredefined())
				continue;
			addref(&e);
		}
		sort(0, true);
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
	int getimage(int index) const override {
		return 2;
	}
public:
	metadata* getcurrent() const {
		if(getmaximum())
			return (metadata*)get(current);
		return 0;
	}
	metadata_control() : plugin("metadata", DockLeft) {
		show_header = false;
		read_only = true;
		no_change_count = true;
		no_change_order = true;
		select_mode = SelectRow;
	}
} metadata_instance;

static class requisit_control : public controls::tableref, controls::control::plugin {
	struct metadata* current_parent;
	static void getpresent(const void* p, stringbuilder& sb) {
		((requisit*)p)->getname(sb);
	}
	void after_initialize() override {
		auto meta = bsmeta<requisit>::meta;
		addstdimage();
		addcol(meta, "id", "Наименование").set(SizeAuto).set(getpresent);
		update();
	}
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
	control& getcontrol() override {
		return *this;
	}
	const char* getlabel(char* result, const char* result_maximum) const override {
		return "Реквизиты";
	}
	void update() {
		clear();
		if(current_parent) {
			for(auto& e : bsdata<requisit>()) {
				if(e.parent==current_parent)
					addref(&e);
			}
		}
	}
	command* getcommands() const override {
		return 0;
	}
	int getimage(int index) const override {
		return 0;
	}
public:
	requisit* getcurrent() const {
		if(getmaximum())
			return (requisit*)get(current);
		return 0;
	}
	void set(metadata* v) {
		if(v == current_parent)
			return;
		current_parent = v;
		update();
	}
	requisit_control() : plugin("requisit", DockLeftBottom) {
		show_header = false;
		read_only = true;
		no_change_count = true;
		no_change_order = true;
		select_mode = SelectRow;
	}
} requisit_instance;

static struct properties_control : controls::properties, controls::control::plugin {
	static void choose_metadata(adat<void*, 64>& result, const bsreq** name_requisit, void* type) {
		if(name_requisit)
			*name_requisit = bsmeta<metadata>::meta;
		for(auto& e : bsdata<metadata>()) {
			if(!e)
				continue;
			if(e.isreference() || e.isarray())
				continue;
			result.add(&e);
		}
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

static void heartproc() {
	requisit_instance.set(metadata_instance.getcurrent());
	bsval v;
	if(isfocused(anyval(&requisit_instance, 0, 0))) {
		v.data = requisit_instance.getcurrent();
		v.type = bsmeta<requisit>::meta;
	} else if(isfocused(anyval(&metadata_instance, 0, 0))) {
		v.data = metadata_instance.getcurrent();
		v.type = bsmeta<metadata>::meta;
	}
	if(v)
		properties_instance.set(v);
}

static void choose_metadata() {
	metadata_instance.setfocus(true);
}

static void choose_requisit() {
	requisit_instance.setfocus(true);
}

static void choose_properties() {
	properties_instance.focusfirst();
}

static shortcut shortcuts[] = {{choose_metadata, "Активировать типы", Ctrl + Alpha + 'T'},
{choose_requisit, "Активировать реквизиты", Ctrl + Alpha + 'R'},
{choose_properties, "Активировать свойства", Ctrl + Alpha + 'P'},
{}};

void run_main() {
	draw::application("Scripter", false, 0, heartproc, shortcuts);
}