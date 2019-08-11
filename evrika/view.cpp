#include "xface/draw_control.h"
#include "xface/draw_grid.h"
#include "main.h"

using namespace draw;

static struct header_control : controls::table, controls::control::plugin {
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
	header_control() : table(), plugin("header", DockLeft) {
		show_header = false;
		read_only = true;
		no_change_count = true;
		no_change_order = true;
		select_mode = SelectRow;
	}
} metadata_instance;

static void pass_verification() {
}

static void test_database() {
}

int main() {
	metadata::initialize();
	draw::application("Evrika", true, pass_verification);
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}