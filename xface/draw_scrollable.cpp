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
	control::view(rcc);
	rect scrollv, scrollh, rc = rcc;
	if(show_border)
		rc.offset(1, 1);
	int rcheight = rc.height();
	int rcwidth = rc.width();
	// calculate scroll positions
	scrollv.clear();
	scrollh.clear();
	if(rcheight < maximum.y) {
		scrollv.set(rc.x2 - metrics::scroll, rc.y1, rc.x2 - 1, rc.y2 - 1);
		rc.x2 -= scrollv.width();
	}
	if(maximum.x && rcwidth < maximum.x - 2) {
		scrollh.set(rc.x1, rc.y2 - metrics::scroll, rc.x2 - 1, rc.y2 - 1);
		rc.y2 -= scrollh.height();
	}
	// mouse events modified scroll behaivor
	auto a = ishilite(rc);
	if(a) {
		int ws;
		switch(hot.key) {
		case MouseWheelUp:
			ws = imin(32 / imin((int)wheels.y, 1), 1);
			if(ishilite(scrollh))
				origin.x -= wheels.x*ws;
			else
				origin.y -= wheels.y*ws;
			invalidate();
			break;
		case MouseWheelDown:
			ws = imin(32 / imin((int)wheels.y, 1), 1);
			if(ishilite(scrollh))
				origin.x += wheels.x*ws;
			else
				origin.y += wheels.y*ws;
			invalidate();
			break;
		}
	}
	// normalize coordinates of scrolls
	if(maximum.y && (origin.y + rcheight > maximum.y))
		origin.y = maximum.y - rcheight;
	if(origin.y < 0)
		origin.y = 0;
	if(maximum.x && (origin.x + rcwidth > maximum.x))
		origin.x = maximum.x - rcwidth;
	if(origin.x < 0)
		origin.x = 0;
	// draw content
	if(true) {
		draw::state push;
		int x1 = rc.x1;
		int y1 = rc.y1;
		setclip({x1, y1, rc.x1 + rcwidth, rc.y1 + rcheight});
		auto push_mouseinput = draw::mouseinput;
		if(hot.mouse.in(scrollh) || hot.mouse.in(scrollv))
			draw::mouseinput = false;
		redraw({x1, y1, rc.x1 + rcwidth, rc.y1 + rcheight});
		draw::mouseinput = push_mouseinput;
	}
	// vertical scroll
	if(scrollv) {
		int current = origin.y;
		draw::scrollv(scrollv, origin.y, rcheight, maximum.y, isfocused());
		if(current != origin.y)
			invalidate();
	}
	// horizontal scroll
	if(scrollh) {
		int current = origin.x;
		draw::scrollh(scrollh, origin.x, rcwidth, maximum.x, isfocused());
		if(current != origin.x)
			invalidate();
	}
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