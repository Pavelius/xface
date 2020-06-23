#include "draw_control.h"
#include "main.h"

using namespace draw;

static struct header_control : controls::tree, controls::control::plugin {
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
	header_control() : plugin("header", DockLeft) {
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

int main() {
	initialize_metadata();
	draw::application("Evrika", true, pass_verification, 0, 0);
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}