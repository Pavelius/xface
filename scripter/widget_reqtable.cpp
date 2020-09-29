#include "main.h"
#include "draw_control.h"

using namespace code;
using namespace draw;

class widget_reqtable : public controls::table {
	const requisit*		source;
	array				source_array;
	widget_reqtable(const requisit* v) : source(v), source_array(v->getlenght()) {
		auto parent_type = v->type;
		for(auto& e : bsdata<requisit>())
			addcol(bsmeta<int>::meta, e.id, e.id);
	}
};

static class plugin_reqtable : controls::control::plugin, controls::control::plugin::builder {
	controls::control* create(const char* url) override {
		return 0;
	}
	bool canopen(const char* ulr) const override {
		return true;
	}
	void destroy(controls::control* p) override {
	}
	controls::control::plugin::builder* getbuilder() override {
		return this;
	}
	controls::control* getcontrol() override {
		return 0;
	}
public:
	plugin_reqtable() : plugin("reqtable", DockWorkspace) {
	}
} plugin_control;