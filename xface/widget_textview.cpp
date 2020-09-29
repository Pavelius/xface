#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

class textview_widget : public scrollable {
	const char*		source;
	const char*		url;
	int				cash_width;
	void redraw(const rect& rc) override {
		if(source)
			textf(rc.x1, rc.y1 - origin.y, rc.width(), source);
	}
	void invalidate() override {
		cash_width = -1;
	}
	void beforeredraw(const rect& rc) override {
		if(!source)
			return;
		if(cash_width == -1) {
			rect rc1 = {0, 0, rc.width(), 0};
			maximum.y = textf(rc1, source, 0);
			wheels.x = 8;
			wheels.y = 8;
		}
	}
	const char* geturl(stringbuilder& sb) const override {
		return url;
	}
public:
	textview_widget() : source(), url(0), cash_width(-1) {}
	textview_widget(const char* url) : source(loadt(url)), url(szdup(url)), cash_width(-1) {}
};

static struct textview_plugin : control::plugin, control::plugin::builder {
	textview_plugin() : plugin("textview", DockBottom) {}
	control* getcontrol() {
		return 0;
	}
	builder* getbuilder() {
		return this;
	}
	control* create(const char* url) override {
		return new textview_widget(url);
	}
	void destroy(control* p) override {
	}
	void getextensions(stringbuilder& sb) override {
		sb.add("Текстовый документ (*.txt)");
		sb.addsz();
		sb.add("*.txt");
		sb.addsz();
	}
} textview_plugin_instance;