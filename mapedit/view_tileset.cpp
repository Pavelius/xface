#include "main.h"
#include "draw_control.h"

using namespace draw;

namespace {
struct control_type : controls::picker, controls::control::plugin {
	tileset* selected;
	control& getcontrol() override {
		return *this;
	}
	const char*	getlabel(char* result, const char* result_maximum) const override {
		return "Список тайлов";
	}
	int getmaximum() const override {
		if(!current_tileset)
			return 0;
		auto ps = current_tileset->getsprite();
		if(!ps)
			return 0;
		return ps->count;
	}
	void row(const rect& rc, int index) override {
		draw::state push; setclip(rc);
		auto ps = selected->getsprite();
		image(rc.x1, rc.y1, ps, index, 0);
		rect r1 = {rc.x1, rc.y1, rc.x2 - 1, rc.y2 - 1};
		auto a = ishilite(r1);
		if(index == current) {
			rectf(r1, colors::edit, 96);
			if(isfocused())
				rectx(r1, colors::text.mix(colors::form, 200));
		}
	}
	void view(const rect& rc) override {
		rect r1 = rc;
		r1.y1 += field(r1.x1, r1.y1, r1.width(), "Набор", current_tileset, 50,
			bsdata<tileset>::source, tileset::getname, 0, 0, 0);
		picker::view(r1);
	}
	void update() {
		selected->read();
		auto ps = selected->getsprite();
		pixels_per_line = ps->height;
		pixels_per_column = ps->width;
		if(pixels_per_line>128)
			pixels_per_line = 128;
		if(pixels_per_column > 128)
			pixels_per_column = 128;
	}
	control_type() : plugin("tile_list", DockRight) {
		show_grid_lines = false;
	}
};
}
static control_type object;

void update_tileset() {
	if(object.selected == current_tileset)
		return;
	object.selected = current_tileset;
	object.update();
}