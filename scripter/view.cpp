#include "setting.h"
#include "draw_control.h"
#include "markup.h"
#include "main.h"

using namespace code;
using namespace draw;

static bool metadata_view_show_type = true;

static class metadata_control : public controls::tableref, controls::control::plugin, initplugin {
	void* addrow() override {
		return 0;
	}
	void after_initialize() override {
		addstdimage();
		addcol("������������", ANREQ(metadata, id), "text").set(SizeAuto);
		update();
	}
	control* getcontrol() override {
		return this;
	}
	const char* getlabel(stringbuilder &sb) const override {
		return "����";
	}
	command* getcommands() const override {
		return 0;
	}
	int getimage(int index) const override {
		return 2;
	}
public:
	void update() {
		clear();
		for(auto& e : bsdata<metadata>()) {
			if(!e)
				continue;
			add(&e);
		}
		sort(1, true);
	}
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
	static const char* getpresent(const void* p, stringbuilder& sb) {
		if(metadata_view_show_type)
			((requisit*)p)->getname(sb);
		else
			((requisit*)p)->getnameonly(sb);
		return sb;
	}
	void after_initialize() override {
		addstdimage();
		addcol("������������", ANREQ(requisit, id), "text").set(SizeAuto).set({getpresent});
		update();
	}
	control* getcontrol() override {
		return this;
	}
	const char* getlabel(stringbuilder& sb) const override {
		return "���������";
	}
	command* getcommands() const override {
		return commands_add;
	}
	int getimage(int index) const override {
		auto p = (requisit*)get(index);
		if(p->is(Method))
			return 4;
		return 0;
	}
	void* addrow() override {
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
		if(current >= getmaximum())
			current = getmaximum() - 1;
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
		if(i == -1)
			return;
		current = i;
	}
	bool change(bool run) override {
		auto p = getcurrent();
		if(!p)
			return false;
		if(run)
			p->edit();
		return true;
	}
	requisit_control() : plugin("requisit", DockLeftBottom) {
		show_header = false;
		show_toolbar = false;
		read_only = true;
		no_change_count = true;
		no_change_order = true;
		select_mode = SelectRow;
	}
} requisit_instance;

static void heartproc() {
	requisit_instance.set(metadata_instance.getcurrent());
	if(isfocused(anyval(&requisit_instance, 0, 0)))
		setproperties(requisit_instance.getcurrent(), dginf<requisit>::meta);
	else if(isfocused(anyval(&metadata_instance, 0, 0)))
		setproperties(metadata_instance.getcurrent(), dginf<metadata>::meta);
}

static void choose_metadata() {
	metadata_instance.setfocus(true);
}

static void choose_requisit() {
	requisit_instance.setfocus(true);
}

static void choose_properties() {
	setpropertiesfocus();
}

static void new_requisit() {
	auto p = metadata_instance.getcurrent();
	if(!p)
		return;
	auto r = p->add("member1", metadata::type_text);
	requisit_instance.update();
	requisit_instance.set(r);
	setproperties(requisit_instance.getcurrent(), dginf<requisit>::meta);
	setpropertiesfocus();
}

static void new_method() {
	auto p = metadata_instance.getcurrent();
	if(!p)
		return;
	auto r = p->add("proc1", metadata::type_text);
	r->flags.add(Method);
	requisit_instance.update();
	requisit_instance.set(r);
	setproperties(requisit_instance.getcurrent(), dginf<requisit>::meta);
	setpropertiesfocus();
}

static void new_type() {
	auto p = bsdata<metadata>::add();
	p->id = szdup("type1");
	metadata_instance.add(p, true);
	setproperties(metadata_instance.getcurrent(), dginf<metadata>::meta);
	setpropertiesfocus();
}

static void remove_requisit() {
	auto p = requisit_instance.getcurrent();
	if(!p)
		return;
	p->clear();
	requisit_instance.update();
	setproperties(requisit_instance.getcurrent(), dginf<requisit>::meta);
	setpropertiesfocus();
}

static class plugin_metadata : controls::control::plugin, controls::control::plugin::builder {
	controls::control* create(const char* url) override {
		return 0;
	}
	bool read(const char* url) override {
		metadata::read(url);
		metadata_instance.update();
		requisit_instance.update();
		return true;
	}
	controls::control::plugin::builder* getbuilder() override {
		return this;
	}
	void getextensions(stringbuilder& sb) const override {
		sb.add("����� ������� (*.mtd)");
		sb.addsz();
		sb.add("*.mtd");
		sb.addsz();
	}
	controls::control* getcontrol() override {
		return 0;
	}
public:
	plugin_metadata() : plugin("metadata_loader", DockWorkspace) {
	}
} plugin_control;

static shortcut shortcuts[] = {{choose_metadata, "������������ ����", Ctrl + 'T'},
{choose_requisit, "������������ ���������", Ctrl + 'R'},
{choose_properties, "������������ ��������", Ctrl + 'P'},
{new_requisit, "�������� ��������", Ctrl + 'N'},
{new_method, "�������� �����", Ctrl + 'M'},
{new_type, "�������� ���", Ctrl + Alt + 'N'},
{}};
controls::control::command requisit_control::commands_add[] = {{"add", "�������� ��������", 0, new_requisit, 9},
{"add_method", "�������� �����", 0, new_method, 9},
{"remove", "�������", 0, remove_requisit, 19, KeyDelete},
{"moveup", "����������� �����", 0, &requisit_control::moveup, 19},
{"movedown", "����������� ����", 0, &requisit_control::movedown, 19},
{}};
static setting::element code_editor_url[] = {{"���� � ����������", {metadata::classes_url, setting::Url}},
{"���� � ��������", {metadata::projects_url, setting::Url}},
};
static setting::element code_editor_metadata[] = {{"���������� ���� � ���� ����������", metadata_view_show_type},
};
static setting::element project_common[] = {{"���", project::main.name},
{"��������", project::main.description},
};
static setting::header headers[] = {{"�������� ����", "����������", 0, code_editor_metadata},
{"�������� ����", "����", 0, code_editor_url},
{"������", "�����", "���������", project_common},
};

static void improt_test() {
	//project::import();
}

void run_main() {
	draw::application("Scripter", improt_test, heartproc, shortcuts);
}