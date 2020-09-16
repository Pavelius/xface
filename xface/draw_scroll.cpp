#include "draw.h"

static int drag_value;

void draw::scrollv(const rect& scroll, int& origin, int count, int maximum, bool focused) {
	// count - elements per page
	// maximum - maximum elements
	if(!maximum || maximum == count)
		return;
	int p1;
	int ss = (scroll.height()*count) / maximum; // scroll size (in pixels)
	int ds = scroll.height() - ss;
	int dr = maximum - count;
	int p = (origin*ds) / dr + scroll.y1;
	auto a = ishilite(scroll);
	auto need_correct = false;
	if(dragactive(&origin)) {
		a = true;
		p1 = hot.mouse.y - drag_value;
		origin = ((p1 - scroll.y1)*dr) / ds;
		need_correct = true;
	} else if(a && hot.pressed && hot.key == MouseLeft) {
		if(hot.mouse.y < p)
			origin -= count;
		else if(hot.mouse.y > p + ss)
			origin += count;
		else {
			dragbegin(&origin);
			drag_value = hot.mouse.y - p;
		}
		need_correct = true;
	}
	// show scroll area
	if(a) {
		if(hot.pressed) {
			rectf({scroll.x1, scroll.y1, scroll.x2 + 1, scroll.y2 + 1}, colors::button, 128);
			buttonv({scroll.x1, p, scroll.x2, p + ss}, true, false, false, true, 0);
		} else {
			rectf({scroll.x1, scroll.y1, scroll.x2 + 1, scroll.y2 + 1}, colors::button, 128);
			buttonv({scroll.x1, p, scroll.x2, p + ss}, false, false, false, true, 0);
		}
	} else {
		if(focused)
			rectf({scroll.x2 - 2, p, scroll.x2 + 2, p + ss}, colors::blue, 128);
	}
	if(need_correct) {
		if(origin < 0)
			origin = 0;
		if(origin + count > maximum)
			origin = maximum - count;
	}
}

void draw::scrollh(const rect& scroll, int& origin, int per_page, int maximum, bool focused) {
	if(!maximum)
		return;
	int p1;
	int ss = ((scroll.x2 - scroll.x1)*per_page) / maximum;
	int ds = scroll.x2 - scroll.x1 - ss;
	int dr = maximum - per_page;
	if(!dr)
		return;
	int p = (origin*ds) / dr + scroll.x1;
	auto a = ishilite(scroll);
	auto need_correct = false;
	if(dragactive(&origin)) {
		a = true;
		p1 = hot.mouse.x - drag_value;
		origin = ((p1 - scroll.x1)*dr) / ds;
		need_correct = true;
	} else if(a && hot.pressed && hot.key == MouseLeft) {
		if(hot.mouse.x < p)
			origin -= per_page;
		else if(hot.mouse.x > p + ss)
			origin += per_page;
		else {
			dragbegin(&origin);
			drag_value = hot.mouse.x - p;
		}
		need_correct = true;
	} else if(a) {
		auto inc = per_page / 10;
		if(inc < 1)
			inc = 1;
		switch(hot.key) {
		case MouseWheelUp:
			origin -= inc;
			need_correct = true;
			break;
		case MouseWheelDown:
			origin += inc;
			need_correct = true;
			break;
		}
	}
	if(need_correct) {
		if(origin < 0)
			origin = 0;
		if(origin + per_page > maximum)
			origin = maximum - per_page;
	}
	if(a) {
		if(hot.pressed) {
			rectf({scroll.x1, scroll.y1, scroll.x2 + 1, scroll.y2 + 1}, colors::button, 128);
			buttonh({p, scroll.y1, p + ss, scroll.y2}, true, false, false, true, 0);
		} else {
			rectf({scroll.x1, scroll.y1, scroll.x2 + 1, scroll.y2 + 1}, colors::button, 128);
			buttonh({p, scroll.y1, p + ss, scroll.y2}, false, false, false, true, 0);
		}
	} else {
		if(focused)
			rectf({p, scroll.y2 - 2, p + ss, scroll.y2 + 2}, colors::blue, 128);
	}
}

draw::scroll::scroll(int& origin, int page, int maximum, const rect& client, bool horizontal) :
	origin(origin), page(page), maximum(maximum), horizontal(horizontal) {
	if(maximum > page) {
		if(horizontal)
			work.set(client.x1, client.y2 - metrics::scroll, client.x2, client.y2);
		else
			work.set(client.x2 - metrics::scroll, client.y1, client.x2, client.y2);
	} else
		work.clear();
}

rect draw::scroll::getslide() const {
	if(horizontal) {
		auto pix_page = work.width();
		auto ss = (pix_page * page) / maximum; // scroll size (in pixels)
		auto ds = pix_page - ss;
		int dr = maximum - page;
		int p = (origin*ds) / dr + work.y1;
		return {p, work.y1, p + ss, work.y2};
	} else {
		auto pix_page = work.height();
		auto ss = (pix_page * page) / maximum; // scroll size (in pixels)
		auto ds = pix_page - ss;
		int dr = maximum - page;
		int p = (origin*ds) / dr + work.y1;
		return {work.x1, p, work.x2, p + ss};
	}
}

void draw::scroll::view(bool focused) {
	if(!isvisible())
		return;
	auto a = draw::ishilite(work);
	if(a) {
		auto scroll = work; scroll.x2++; scroll.y2++;
		auto slide = getslide();
		if(hot.pressed) {
			draw::rectf(scroll, colors::button, 128);
			draw::buttonh(slide, true, false, false, true, 0);
		} else {
			draw::rectf(scroll, colors::button, 128);
			draw::buttonh(slide, false, false, false, true, 0);
		}
	} else {
		if(focused) {
			auto slide = getslide();
			draw::rectf({slide.x1, work.y2 - 2, slide.x2, work.y2 + 2}, colors::blue, 128);
		}
	}
}

void draw::scroll::input() {
	if(!isvisible())
		return;
	auto a = draw::ishilite(work);
	auto need_correct = false;
	auto pix_page = work.width();
	auto ss = (pix_page * page) / maximum; // scroll size (in pixels)
	auto ds = pix_page - ss;
	/*if(dragactive(&origin)) {
		a = true;
		p1 = hot.mouse.y - drag_value;
		auto dr = maximum - page;
		origin = ((p1 - work.y1)*dr) / ds;
		need_correct = true;
	} else if(a && hot.pressed && hot.key == MouseLeft) {
		if(hot.mouse.y < p)
			origin -= count;
		else if(hot.mouse.y > p + ss)
			origin += count;
		else {
			dragbegin(&origin);
			drag_value = hot.mouse.y - p;
		}
		need_correct = true;
	}*/
}