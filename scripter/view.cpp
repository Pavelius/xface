#include "xface/crt.h"
#include "xface/draw_properties.h"
#include "xface/setting.h"
#include "main.h"

using namespace code;
using namespace draw;

static bool metadata_view_show_type = true;

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
			add(&e);
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
	void add(const metadata* v, bool interactive = false) {
		if(!v || v->isreference() || v->isarray() || v->ispredefined())
			return;
		addref(const_cast<metadata*>(v));
		if(interactive) {
			sort(0, true);
			auto i = tableref::find(v);
			if(i != -1) {
				current = i;
				ensurevisible();
			}
		}
	}
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
	static command commands_add[];
	static const char* getpresent(const void* p, char* result, const char* result_maximum) {
		stringbuilder sb(result, result_maximum);
		if(metadata_view_show_type)
			((requisit*)p)->getname(sb);
		else
			((requisit*)p)->getnameonly(sb);
		return result;
	}
	void after_initialize() override {
		auto meta = bsmeta<requisit>::meta;
		addstdimage();
		addcol(meta, "id", "Наименование").set(SizeAuto).set(getpresent);
		update();
	}
	control& getcontrol() override {
		return *this;
	}
	const char* getlabel(char* result, const char* result_maximum) const override {
		return "Реквизиты";
	}
	command* getcommands() const override {
		return commands_add;
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
	void update() {
		clear();
		if(current_parent) {
			for(auto& e : bsdata<requisit>()) {
				if(e.parent == current_parent)
					addref(&e);
			}
		}
	}
	int find(const requisit* v) const {
		return tableref::find(v);
	}
	void set(metadata* v) {
		if(v == current_parent)
			return;
		current_parent = v;
		update();
	}
	void set(const requisit* v) {
		auto i = find(v);
		if(i==-1)
			return;
		current = i;
	}
	bool addrow(bool run);
	bool removerow(bool run);
	requisit_control() : plugin("requisit", DockLeftBottom) {
		show_header = false;
		show_toolbar = false;
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

static void new_requisit() {
	auto p = metadata_instance.getcurrent();
	if(!p)
		return;
	auto r = p->add("New1", metadata::type_text);
	requisit_instance.update();
	requisit_instance.set(r);
	properties_instance.set(requisit_instance.getcurrent(), bsmeta<requisit>::meta);
	properties_instance.focusfirst();
}

static void new_type() {
	auto p = bsdata<metadata>::add();
	p->id = szdup("Type1");
	metadata_instance.add(p, true);
	properties_instance.set(metadata_instance.getcurrent(), bsmeta<metadata>::meta);
	properties_instance.focusfirst();
}

static void remove_requisit() {
}

static shortcut shortcuts[] = {{choose_metadata, "Активировать типы", Ctrl + Alpha + 'T'},
{choose_requisit, "Активировать реквизиты", Ctrl + Alpha + 'R'},
{choose_properties, "Активировать свойства", Ctrl + Alpha + 'P'},
{new_requisit, "Добавить реквизит", Ctrl + Alpha + 'N'},
{new_type, "Добавить тип", Ctrl + Alt + Alpha + 'N'},
{}};

bool requisit_control::addrow(bool run) {
	if(run)
		new_requisit();
	return true;
}

bool requisit_control::removerow(bool run) {
	if(run)
		remove_requisit();
	return true;
}

controls::control::command requisit_control::commands_add[] = {{"add", "Добавить", 0, &requisit_control::addrow, 9},
{"remove", "Удалить", 0, &table::removerow, 19, KeyDelete},
{}};

static setting::element code_editor_metadata[] = {{"Показывать типы в окне метаданных", metadata_view_show_type},
};
static setting::header headers[] = {{"Редактор кода", "Метаданные", 0, code_editor_metadata},
};

void run_main() {
	draw::application("Scripter", false, 0, heartproc, shortcuts);
}