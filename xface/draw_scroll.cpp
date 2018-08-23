#include "draw.h"

void draw::scrollv(int id, const rect& scroll, int& origin, int count, int maximum, bool focused) {
	// count - elements per page
	// maximum - maximum elements
	if(!maximum || maximum == count)
		return;
	int p1;
	int ss = (scroll.height()*count) / maximum; // scroll size (in pixels)
	int ds = scroll.height() - ss;
	int dr = maximum - count;
	int p = (origin*ds) / dr + scroll.y1;
	auto a = area(scroll);
	if(drag::active(id, DragScrollV)) {
		a = AreaHilitedPressed;
		p1 = hot::mouse.y - drag::value;
		origin = ((p1 - scroll.y1)*dr) / ds;
		if(origin < 0)
			origin = 0;
		if(origin + count > maximum)
			origin = maximum - count;
	} else if(a == AreaHilitedPressed && hot::key == MouseLeft) {
		if(hot::mouse.y < p)
			origin -= count;
		else if(hot::mouse.y > p + ss)
			origin += count;
		else {
			drag::begin(id, DragScrollV);
			drag::value = hot::mouse.y - p;
		}
		if(origin < 0)
			origin = 0;
		if(origin + count > maximum)
			origin = maximum - count;
	}
	// show scroll area
	switch(a) {
	case AreaHilited:
		rectf({scroll.x1, scroll.y1, scroll.x2 + 1, scroll.y2 + 1}, colors::button, 128);
		buttonv({scroll.x1, p, scroll.x1 + scroll.width(), p + ss}, false, false, false, true, 0);
		break;
	case AreaHilitedPressed:
		rectf({scroll.x1, scroll.y1, scroll.x2 + 1, scroll.y2 + 1}, colors::button, 128);
		buttonv({scroll.x1, p, scroll.x1 + scroll.width(), p + ss}, true, false, false, true, 0);
		break;
	default:
		if(focused)
			rectf({scroll.x2 - 2, p, scroll.x2 + 2, p + ss}, colors::blue, 128);
		break;
	}
}

void draw::scrollh(int id, const struct rect& scroll, int& origin, int count, int maximum, bool focused) {
	if(!maximum)
		return;
	int p1;
	int ss = ((scroll.x2 - scroll.x1)*count) / maximum;
	int ds = scroll.x2 - scroll.x1 - ss;
	int dr = maximum - count;
	if(!dr)
		return;
	int p = (origin*ds) / dr + scroll.x1;
	areas a = area(scroll);
	if(drag::active(id, DragScrollH)) {
		a = AreaHilitedPressed;
		p1 = hot::mouse.x - drag::value;
		origin = ((p1 - scroll.x1)*dr) / ds;
		if(origin < 0)
			origin = 0;
		if(origin + count > maximum)
			origin = maximum - count;
	} else if(a == AreaHilitedPressed && hot::key == MouseLeft) {
		if(hot::mouse.x < p)
			origin -= count;
		else if(hot::mouse.x > p + ss)
			origin += count;
		else {
			drag::begin(id, DragScrollH);
			drag::value = hot::mouse.x - p;
		}
		if(origin < 0)
			origin = 0;
		if(origin + count > maximum)
			origin = maximum - count;
	}
	switch(a) {
	case AreaHilited:
		rectf({scroll.x1, scroll.y1, scroll.x2 + 1, scroll.y2 + 1}, colors::button, 128);
		buttonh({p, scroll.y1, ss, scroll.height()}, false, false, false, true, 0);
		break;
	case AreaHilitedPressed:
		rectf({scroll.x1, scroll.y1, scroll.x2 + 1, scroll.y2 + 1}, colors::button, 128);
		buttonh({p, scroll.y1, ss, scroll.height()}, true, false, false, true, 0);
		break;
	default:
		if(focused)
			rectf({p, scroll.y2 - 2, p + ss, scroll.y2 + 2}, colors::blue, 128);
		break;
	}
}