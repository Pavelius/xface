#include "draw_control.h"
#include "main.h"

using namespace draw;

static struct header_control : controls::tree, controls::control::plugin {
	struct element : tree::element {
		unsigned	rfid;
	};
	void before_render() {
	}
	control* getcontrol() override {
		return this;
	}
	const char* getlabel(stringbuilder& sb) const override {
		return "Типы";
	}
	command* getcommands() const override {
		return 0;
	}
	void expanding(int index, int level) override {
		if(!level) {
			for(auto& e : databases[HeaderType].records<header>()) {
				auto pn = (element*)addrow();
				pn->level = 1;
				pn->setgroup(true);
			}
		} else {
			auto pc = (element*)get(index);
			auto pn = (element*)insert(index, level + 1);
			pn->setgroup(true);
		}
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
	metadata_instance.expanding(0, 0);
	draw::application("Evrika", true, pass_verification, 0, 0);
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}