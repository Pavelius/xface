#include "crt.h"
#include "draw_control.h"

using namespace draw::controls;

scrollable::scrollable() {
	origin.x = 0;
	origin.y = 0;
	maximum.x = 0;
	maximum.y = 0;
	wheels.x = 1;
	wheels.y = 1;
}

void scrollable::invalidate() {
	maximum.x = 0;
}

void scrollable::view(const rect& rcc) {
	rect rc = rcc;
	if(show_border)
		rc.offset(1, 1);
	draw::scroll scrollv(origin.y, rc.height(), maximum.y, rc, false, wheels.y);
	draw::scroll scrollh(origin.x, rc.width(), maximum.x, rc, true, wheels.x);
	scrollv.correct();
	scrollh.correct();
	scrollv.input();
	scrollh.input();
	control::view(rcc);
	beforeredraw(rc);
	if(true) {
		draw::state push;
		setclip(rc);
		redraw(rc);
	}
	scrollv.view(isfocused());
	scrollh.view(isfocused());
}

rect draw::controls::scrollable::centerview(const rect& rc) {
	rect rs = rc;
	if(rc.width() > maximum.x) {
		rs.x1 = rc.x1 + (rc.width() - maximum.x) / 2;
		rs.x2 = rs.x1 + maximum.x;
	}
	if(rc.height() > maximum.y) {
		rs.y1 = rc.y1 + (rc.height() - maximum.y) / 2;
		rs.y2 = rs.y1 + maximum.y;
	}
	return rs;
}