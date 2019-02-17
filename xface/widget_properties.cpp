#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

static bsval(*callback_autoset)();
static unsigned(*callback_getflags)(const bsval& ev);

namespace draw::controls {

struct properties : control, control::plugin {

	int			title, spacing;
	bsval		value;

	int vertical(int x, int y, int width, const bsval& ev) {
		if(!ev)
			return 0;
		int y0 = y;
		for(auto p = ev.type; *p; p++)
			y += element(x, y, width, bsval(ev.data, p));
		return y - y0;
	}

	int element(int x, int y, int width, const bsval& ev) {
		storage st;
		st.data = (void*)ev.type->ptr(ev.data);
		st.size = ev.type->size;
		int h = 0;
		unsigned flags = 0;
		if(callback_getflags)
			flags = callback_getflags(ev);
		if(ev.type->type == number_type) {
			st.type = st.Number;
			h = field(x, y, width, flags, st, ev.type->id, 0, title);
		} else if(ev.type->type == text_type) {
			st.type = st.TextPtr;
			h = field(x, y, width, flags, st, ev.type->id, 0, title);
		} else if(ev.type->reference>0) {
			h = combobox(x, y, width, flags, ev, ev.type->id, 0, title);
		}
		if(!h)
			return 0;
		return h + spacing;
	}

	void view(const rect& rc) override {
		if(callback_autoset) {
			auto v = callback_autoset();
			if(v)
				value = v;
		}
		control::view(rc);
		if(!value) {
			auto push_fore = fore;
			fore = colors::text.mix(colors::window, 128);
			text(rc, "У объекта нет свойств", AlignCenterCenter);
			fore = push_fore;
		} else {
			auto x = rc.x1;
			auto y = rc.y1;
			auto width = rc.width();
			setposition(x, y, width);
			vertical(x, y, width, value);
		}
	}

	control& getcontrol() override {
		return *this;
	}

	const char* getlabel(char* result, const char* result_maximum) const override {
		return "Свойства";
	}

	bool isfocusable() const override {
		return false;
	}

public:

	void set(const bsval& value) {
		this->value = value;
	}

	void clear() {
		value.data = 0;
		value.type = 0;
	}

	properties() : plugin("properties", DockRight), title(80), spacing(0) {
		show_background = false;
	}

};
}

static properties properties_control;

void propset(const bsval& value) {
	properties_control.set(value);
}

void propclear() {
	properties_control.clear();
}

void propset(bsval proc()) {
	callback_autoset = proc;
}

void propset(unsigned(*proc)(const bsval& ev)) {
	callback_getflags = proc;
}