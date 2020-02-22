#include "draw_control.h"
#include "main.h"

using namespace draw;

static struct header_control : controls::tableref, controls::control::plugin {
	void before_render() {
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
	void expanding(builder& e) {
		if(e.level == 0) {
			e.add(0, 1, 0, true);
			e.add(0, 1, 0, true);
			e.add(0, 1, 0, true);
			e.add(0, 1, 0, true);
		} else {
		}
	}
	header_control() : tableref(), plugin("header", DockLeft) {
		show_header = false;
		read_only = true;
		no_change_count = true;
		no_change_order = true;
		select_mode = SelectRow;
	}
} metadata_instance;

static void pass_verification() {
}

void initialize_metadata();

static void test_database() {
	auto p = databases[UserType].add();
	auto r = (requisit*)databases[UserType].requisits.get(0);
	r->set(p, "firstname", "Павел");
	r->set(p, "surname", "Чистяков");
	r->set(p, "lastname", "Валентинович");
}

int main() {
	initialize_metadata();
	test_database();
	metadata::writefile("test.edb");
	draw::application("Evrika", true, pass_verification);
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}