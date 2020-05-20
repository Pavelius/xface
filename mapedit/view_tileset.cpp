#include "main.h"
#include "draw_control.h"

using namespace draw;

struct tileset_control_type : controls::list, controls::control::plugin {
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
	void view(const rect& rc) override {
		rect r1 = rc;
		r1.y1 += field(r1.x1, r1.y1, r1.width(), "Набор", current_tileset, 50,
			bsdata<tileset>::source, tileset::getname, 0, 0, 0);
		list::view(r1);
	}
	void update() {
		selected->read();
		auto ps = selected->getsprite();
		pixels_per_line = 64;
		pixels_per_width = 64;
	}
	tileset_control_type() : plugin("tile_list", DockRight) {
		show_grid_lines = false;
	}
};
static tileset_control_type object;

void update_tileset() {
	if(object.selected == current_tileset)
		return;
	object.selected = current_tileset;
	object.update();
}