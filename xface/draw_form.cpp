#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

void form::field(const char* label, const anyval& av) {
	rc.y1 += draw::field(rc.x1, rc.y1, rc.width(), label, av, title, 5);
}

void form::field(control& ev) {
	ev.view(rc);
}

int form::edit() {
	while(ismodal()) {
		rect rc = {0, 0, draw::getwidth(), draw::getheight()};
		draw::rectf(rc, colors::form);
		if(metrics::show::statusbar)
			rc.y2 -= draw::statusbardw();
		if(metrics::show::padding) {
			rc.x1 += metrics::padding;
			rc.x2 -= metrics::padding;
			rc.y1 += metrics::padding;
		}
		domodal();
	}
	return getresult();
}