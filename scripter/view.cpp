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

static class metadata_control : public controls::tableref, controls::control::plugin, initplugin {
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

static class requisit_control : public controls::tableref, controls::control::plugin, initplugin {
	struct metadata* current_parent;
	static const char* getpresent(const void* p, char* result, const char* result_maximum) {
		stringbuilder sb(result, result_maximum);
		((requisit*)p)->getname(sb);
		return result;
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
	static bool choose_metadata(const void* object, int value) {
		auto p = &bsdata<metadata>::elements[value];
		if(value && (p->isarray() || p->isreference()))
			return false;
		return true;
	}
	static const char* gettypename(const void* object, char* result, const char* result_maximum) {
		if(!object)
			return "Неопределено";
		stringbuilder sb(result, result_maximum);
		((metadata*)object)->add(sb);
		return result;
	}
	fnallow getfnallow(const void* object, const bsreq* type) const override {
		if(type->type == bsmeta<metadata>::meta)
			return choose_metadata;
		return 0;
	}
	fntext getfntext(const void* object, const bsreq* type) const override {
		if(type->type == bsmeta<metadata>::meta)
			return gettypename;
		return controls::table::getenumname;
	}
	control& getcontrol() override {
		return *this;
	}
	const char* getlabel(char* result, const char* result_maximum) const override {
		return "Свойства";
	}
	bool isvisible(const void* object, const bsreq* type) const override {
		if(!object)
			return false;
		if(strcmp(type->id, "parent") == 0)
			return false;
		if(strcmp(type->id, "offset") == 0)
			return false;
		if(strcmp(type->id, "size") == 0)
			return false;
		return true;
	}
	properties_control() : plugin("properties", DockRight) {
		title = 100;
		dictionary = translate_data;
	}
} properties_instance;

static void heartproc() {
	requisit_instance.set(metadata_instance.getcurrent());
	if(isfocused(anyval(&requisit_instance, 0, 0)))
		properties_instance.set(requisit_instance.getcurrent(), bsmeta<requisit>::meta);
	else if(isfocused(anyval(&metadata_instance, 0, 0)))
		properties_instance.set(metadata_instance.getcurrent(), bsmeta<metadata>::meta);
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