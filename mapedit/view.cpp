#include "xface/draw_control.h"
#include "xface/draw_grid.h"
#include "xface/widget.h"
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
		switch(type) {
		case IsometricRectangle:
			result.x = pt.x * element.x + element.x / 2 - (pt.x % 2)*element.x/2;
			result.y = (pt.y + 1) * element.y / 2 + element.y / 2;
			break;
		default:
			result.x = pt.x * element.x + element.x / 2;
			result.y = pt.y * element.y + element.y / 2;
			break;
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
		auto p0 = m2s(rc, {0, 0});
		auto p1 = m2s(rc, {(short)size.x - 1, (short)size.y - 1});
		auto x0 = p0.x - element.x / 2;
		auto y0 = p0.y - element.y / 2;
		auto x1 = p1.x + element.x / 2;
		auto y1 = p1.y + element.y / 2;
		auto sx = element.x / 2;
		auto sy = element.y / 2;
		fore = colors::gray.mix(colors::window);
		for(auto y = 0; y < size.y; y++) {
			for(auto x = 0; x < size.x; x++) {
				auto pt = m2s(rc, {(short)x, (short)y});
				pt.x -= sx;
				pt.y -= sy;
				line(pt.x, y0, pt.x, y1);
				line(x0, pt.y, x1, pt.y);
			}
		}
		line(x1, y0, x1, y1);
		line(x0, y1, x1, y1);
		fore = push_fore;
	}

	void redraw(rect rc) override {
		render_grid(rc);
	}

	map_control_type() : plugin("map", DockWorkspace) {
		type = Rectangle;
		show_grid = true;
		element.x = 128;
		element.y = 64;
		size.x = 16;
		size.y = 16;
		maximum.x = size.x * element.x;
		maximum.y = size.y * element.y;
	}

};
static map_control_type map_control;

void run_main() {
	draw::application("X-Map editor", false);
}