#include "draw.h"

using namespace draw;

static int					drag_value;
static draw::scroll::proc	call_proc;
static draw::scroll			call_object;

scroll::scroll(int& origin, int page, int maximum, const rect& client, bool horizontal, int wheel) :
	origin(&origin), page(page), maximum(maximum), wheel(wheel), client(client), horizontal(horizontal) {
	if(maximum > page) {
		if(horizontal)
			work.set(client.x1, client.y2 - metrics::scroll, client.x2, client.y2);
		else
			work.set(client.x2 - metrics::scroll, client.y1, client.x2, client.y2);
	} else
		work.clear();
}

bool scroll::ishilite() const {
	return draw::ishilite(work);
}

void scroll::correct() {
	if(!origin)
		return;
	if(*origin + page > maximum)
		*origin = maximum - page;
	if(*origin < 0)
		*origin = 0;
}

void scroll::callback() {
	(call_object.*call_proc)(hot.param);
}

void scroll::execute(draw::scroll::proc p, int param) const {
	call_proc = p;
	call_object = *this;
	draw::execute(callback, param);
}

rect scroll::getslide() const {
	if(!origin)
		return {0, 0, 0, 0};
	if(horizontal) {
		auto pix_page = work.width();
		auto ss = (pix_page * page) / maximum; // scroll size (in pixels)
		auto ds = pix_page - ss;
		auto dr = maximum - page;
		auto p = ((*origin)*ds) / dr + work.x1;
		return {p, work.y1, p + ss, work.y2};
	} else {
		auto pix_page = work.height();
		auto ss = (pix_page * page) / maximum; // scroll size (in pixels)
		auto ds = pix_page - ss;
		auto dr = maximum - page;
		auto p = ((*origin)*ds) / dr + work.y1;
		return {work.x1, p, work.x2, p + ss};
	}
}

void scroll::view(bool focused) {
	if(!isvisible())
		return;
	auto a = ishilite();
	if(dragactive(origin))
		a = true;
	if(a) {
		auto slide = getslide();
		rectf(work, colors::button, 128);
		if(horizontal)
			buttonh(slide, hot.pressed, false, false, true, 0);
		else
			buttonv(slide, hot.pressed, false, false, true, 0);
	} else {
		if(focused) {
			auto slide = getslide();
			if(horizontal)
				rectf({slide.x1, slide.y2 - 4, slide.x2, slide.y2}, colors::blue, 128);
			else
				rectf({slide.x2 - 4, slide.y1, slide.x2, slide.y2}, colors::blue, 128);
		}
	}
}

void scroll::input() {
	if(!isvisible())
		return;
	if(dragactive(origin)) {
		int p1, pix_page;
		if(horizontal) {
			p1 = hot.mouse.x - drag_value - work.x1;
			pix_page = work.width();
		} else {
			p1 = hot.mouse.y - drag_value - work.y1;
			pix_page = work.height();
		}
		auto pix_scroll = (pix_page * page) / maximum; // scroll size (in pixels)
		auto dr = maximum - page;
		auto ds = pix_page - pix_scroll;
		*origin = (p1*dr) / ds;
		correct();
		//execute(&scroll::setorigin, (p1*dr) / ds);
	} else {
		switch(hot.key) {
		case MouseLeft:
			if(ishilite()) {
				if(hot.pressed) {
					auto slider = getslide();
					if(horizontal) {
						if(hot.mouse.x < slider.x1)
							execute(&scroll::setorigin, *origin - page);
						else if(hot.mouse.x > slider.x2)
							execute(&scroll::setorigin, *origin + page);
						else {
							dragbegin(origin);
							drag_value = hot.mouse.x - slider.x1;
						}
					} else {
						if(hot.mouse.y < slider.y1)
							execute(&scroll::setorigin, *origin - page);
						else if(hot.mouse.y > slider.y2)
							execute(&scroll::setorigin, *origin + page);
						else {
							dragbegin(origin);
							drag_value = hot.mouse.y - slider.y1;
						}
					}
				}
				hot.key = 0;
			}
			break;
		case MouseWheelUp:
			if(draw::ishilite(client))
				execute(&scroll::setorigin, *origin - wheel);
			break;
		case MouseWheelDown:
			if(draw::ishilite(client))
				execute(&scroll::setorigin, *origin + wheel);
			break;
		}
	}
}