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

struct map_control_type : controls::scrollable, mapi, controls::control::plugin {

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
		result.x += rc.x1 - offset.x;
		result.y += rc.y1 - offset.y;
		return result;
	}

	point getoffset(const rect& rc) const {
		point result = {};
		if(size.x == 0 || size.y == 0)
			return result;
		switch(type) {
		case Rectangle:
			result.x = rc.width() / element.x;
			result.y = rc.height() / element.y;
			break;
		}
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
		fore = colors::gray.mix(colors::window, 32);
		auto offset = getoffset(rc);
		point p1, p2;
		for(auto y = 0; y < size.y; y++) {
			p1 = m2s(rc, {(short)0, (short)y});
			p2 = m2s(rc, {(short)size.x, (short)y});
			p1.x -= element.x / 2;
			p1.y -= element.y / 2;
			p2.x -= element.x / 2;
			p2.y -= element.y / 2;
			line(p1.x , p1.y + element.y, p2.x, p2.y + element.y);
		}
		//line(p1.x, p1.y + element.y, p2.x, p2.y + element.y);
		for(auto x = 0; x < size.x; x++) {
			p1 = m2s(rc, {(short)x, (short)0});
			p2 = m2s(rc, {(short)x, (short)size.y});
			p1.x -= element.x / 2;
			p1.y -= element.y / 2;
			p2.x -= element.x / 2;
			p2.y -= element.y / 2;
			line(p1.x, p1.y, p2.x, p2.y);
		}
		//line(p1.x + element.x, p1.y, p2.x + element.x, p2.y);
		fore = push_fore;
	}

	void redraw(const rect& rc) override {
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
		size.y = 4;
		change_type();
	}

};
static map_control_type map_control;

void run_main() {
	draw::application("X-Map editor", false);
}