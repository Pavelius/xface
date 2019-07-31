#include "xface/draw_control.h"
#include "xface/draw_grid.h"
#include "main.h"

static bool		show_grid;
static int		grid_size;

using namespace draw;

void logmsg(const char* format, ...);
void propset(const bsval& value);
void propclear();

void tileset::import() {
	const int d = metrics::padding * 2;
	while(ismodal()) {
		domodal();
	}
}

struct map_control_type : controls::scrollable, map_info, controls::control::plugin {

	control& getcontrol() override {
		return *this;
	}

	point m2s(const rect& rc, const point pt) const {
		point result;
		auto center = true;
		switch(type) {
		case IsometricRectangle:
			result.x = (pt.x/2) * element.x + (pt.x % 2)*element.x / 2;
			result.y = pt.y * element.y / 2;
			break;
		default:
			result.x = pt.x * element.x;
			result.y = pt.y * element.y;
			break;
		}
		if(center) {
			result.x += element.x / 2;
			result.y += element.y / 2;
		}
		result.x += rc.x1;
		result.y += rc.y1;
		return result;
	}

	const char*	getlabel(char* result, const char* result_maximum) const override {
		return "Редактор карт";
	}

	point getcamera() const {
		return {(short)origin.x, (short)origin.y};
	}

	void render_grid(const rect& rc) const {
		auto push_fore = fore;
		fore = colors::gray.mix(colors::window);
		for(auto y = 0; y < size.y; y++) {
			for(auto x = 0; x < size.x; x++) {
				auto pt = m2s(rc, {(short)x, (short)y});
			}
		}
		fore = push_fore;
	}

	void redraw(rect rc) override {
		render_grid(rc);
	}

	void change_type() {
		maximum.x = size.x * element.x;
		maximum.y = size.y * element.y;
	}

	map_control_type() : plugin("map", DockWorkspace) {
		type = IsometricRectangle;
		show_grid = true;
		element.x = 128;
		element.y = 64;
		size.x = 2;
		size.y = 1;
		change_type();
	}

};
static map_control_type map_control;

void run_main() {
	draw::application("X-Map editor", false);
}