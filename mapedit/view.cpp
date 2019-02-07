#include "xface/draw_control.h"
#include "xface/draw_grid.h"
#include "main.h"

static bool		show_grid;
static int		grid_size;

using namespace draw;

void logmsg(const char* format, ...);
void propset(const bsval& value);
void propclear();

struct map_control_type : controls::scrollable, map_info, controls::control::plugin {

	control& getcontrol() override {
		return *this;
	}

	const char*	getlabel(char* result, const char* result_maximum) const override {
		return "Редактор карт";
	}

	point getcamera() const {
		return {(short)origin.x, (short)origin.y};
	}

	point m2s(const rect& rc, const point pt) const {
		point result;
		result.x = pt.x * element.x + rc.x1;
		result.y = pt.y * element.y + rc.y1;
		return result;
	}

	void render_grid(const rect& rc) const {
		auto push_fore = fore;
		fore = colors::gray.mix(colors::window);
		for(auto y = 0; y < size.y; y++) {
			for(auto x = 0; x < size.x; x++) {
				auto pt = m2s(rc, {(short)x, (short)y});
				line(pt.x, rc.y1, pt.x, rc.y2);
				line(rc.x1, pt.y, rc.x2, pt.y);
			}
		}
		fore = push_fore;
	}

	void redraw(rect rc) override {
		render_grid(rc);
	}

	map_control_type() : plugin("map", DockWorkspace) {
		show_grid = true;
		element.x = 32;
		element.y = 32;
		size.x = 16;
		size.y = 16;
	}

};
static map_control_type map_control;

void run_main() {
	application_initialize();
	setcaption("X-Map editor");
	application();
}