#include "codeview.h"

using namespace draw;
using namespace draw::controls;

static struct widget_codeview : codeview, control::plugin, initplugin {

	control& getcontrol() override {
		return *this;
	}

	const char* getlabel(stringbuilder& sb) const override {
		return "Редактор кода";
	}

	widget_codeview() : control::plugin("codeview", DockWorkspace) {
	}

} widget_control;

codemodel& getcurrentmodel() {
	return widget_control;
}