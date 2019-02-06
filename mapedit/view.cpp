#include "xface/draw_control.h"
#include "xface/draw_grid.h"
#include "main.h"

using namespace draw;

void logmsg(const char* format, ...);
void propset(const bsval& value);
void propclear();

struct map_view : controls::scrollable, map_info {

	void redraw(rect rc) override {

	}

};

void run_main() {
	application_initialize();
	setcaption("X-Map editor");
	application();
}