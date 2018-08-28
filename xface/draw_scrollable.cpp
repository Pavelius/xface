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

void scrollable::view(rect rc) {
	struct rect scrollv;
	struct rect scrollh;
	int rcheight = rc.height();
	int rcwidth = rc.width();
	control::view(rc);
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
	auto a = area(rc);
	if(a == AreaHilited) {
		int ws;
		switch(hot.key) {
		case MouseWheelUp:
			ws = imin(32 / imin((int)wheels.y, 1), 1);
			if(areb(scrollh))
				origin.x -= wheels.x*ws;
			else
				origin.y -= wheels.y*ws;
			invalidate();
			break;
		case MouseWheelDown:
			ws = imin(32 / imin((int)wheels.y, 1), 1);
			if(areb(scrollh))
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
		if(hot.mouse.in(scrollh) || hot.mouse.in(scrollv))
			draw::mouseinput = false;
		redraw({x1, y1, rc.x1 + rcwidth, rc.y1 + rcheight});
	}
	// vertical scroll
	if(scrollv) {
		int current = origin.y;
		draw::scrollv((int)this, scrollv, current, rcheight, maximum.y, isfocused());
		if(current != origin.y) {
			origin.y = current;
			invalidate();
		}
	}
	// horizontal scroll
	if(scrollh) {
		int current = origin.x;
		draw::scrollh((int)this, scrollh, current, rcwidth, maximum.x, isfocused());
		if(current != origin.x) {
			origin.x = current;
			invalidate();
		}
	}
}

rect draw::controls::scrollable::centerview(rect rc) {
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