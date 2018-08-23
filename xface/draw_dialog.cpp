#include "crt.h"
#include "draw.h"

using namespace draw;

void draw::setposition(int& x, int& y, int width) {
	x += metrics::padding;
	y -= metrics::padding;
	width -= metrics::padding * 2;
}

void draw::focusing(int id, unsigned& flags, rect rc) {
	if(flags&Disabled)
		return;
	addelement(id, rc);
	if(!getfocus())
		setfocus(id, true);
	if(getfocus() == id)
		flags |= Focused;
	else if(area(rc) == AreaHilitedPressed && hot::key == MouseLeft && hot::pressed) {
		setfocus(id, false);
		hot::key = MouseLeft;
	}
}

int	draw::button(int x, int y, int width, int id, unsigned flags, const char* label, const char* tips, void(*callback)()) {
	setposition(x, y, width);
	struct rect rc = {x, y, x + width, y + 4 * 2 + draw::texth()};
	focusing(id, flags, rc);
	if(buttonh({x, y, x + width, rc.y2},
		ischecked(flags), isfocused(flags), isdisabled(flags), true, label, 0, false, tips)
		|| (isfocused(flags) && hot::key == KeyEnter)) {
		execute(callback, id);
	}
	if(label && label[0] && areb(rc))
		tooltips(tips);
	return rc.height() + metrics::padding * 2;
}

int draw::radio(int x, int y, int width, int id, unsigned flags, const char* label, const char* tips, void(*callback)()) {
	draw::state push;
	setposition(x, y, width);
	rect rc = {x, y, x + width, y};
	rect rc1 = {rc.x1 + 22, rc.y1, rc.x2, rc.y2};
	draw::textw(rc1, label);
	rc1.offset(-2);
	rc.y1 = rc1.y1;
	rc.y2 = rc1.y2;
	rc.x2 = rc1.x2;
	decortext(flags);
	focusing(id, flags, rc);
	clipart(x + 2, y + imax((rc1.height() - 14) / 2, 0), width, flags, ":radio");
	bool need_select = false;
	auto a = draw::area(rc);
	if((a == AreaHilited || a == AreaHilitedPressed) && !isdisabled(flags) && hot::key == MouseLeft) {
		if(!hot::pressed)
			need_select = true;
	}
	if(isfocused(flags)) {
		draw::rectx({rc1.x1, rc1.y1, rc1.x2, rc1.y2}, draw::fore);
		if(!isdisabled(flags) && hot::key == KeySpace)
			need_select = true;
	}
	if(need_select)
		execute(callback, id);
	rc = rc1; rc.offset(2);
	draw::text(rc, label);
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc1.height() + metrics::padding * 2;
}

int draw::checkbox(int x, int y, int width, int id, unsigned flags, const char* label, const char* tips, void(*callback)()) {
	draw::state push;
	setposition(x, y, width);
	rect rc = {x, y, x + width, y};
	rect rc1 = {rc.x1 + 22, rc.y1, rc.x2, rc.y2};
	draw::textw(rc1, label);
	rc.y1 = rc1.y1;
	rc.y2 = rc1.y2;
	rc.x2 = rc1.x2;
	focusing(id, flags, rc);
	clipart(x + 2, y + imax((rc1.height() - 14) / 2, 0), 0, flags, ":check");
	decortext(flags);
	auto a = draw::area(rc);
	auto need_value = false;
	if((a == AreaHilited || a == AreaHilitedPressed) && !isdisabled(flags) && hot::key == MouseLeft) {
		if(!hot::pressed)
			need_value = true;
	}
	if(isfocused(flags)) {
		draw::rectx({rc1.x1 - 2, rc1.y1 - 1, rc1.x2 + 2, rc1.y2 + 1}, draw::fore);
		if(hot::key == KeySpace)
			need_value = true;
	}
	if(need_value)
		execute(callback, id);
	draw::text(rc1, label);
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc1.height() + metrics::padding * 2;
}

bool draw::dodialog(int id) {
	for(auto p = renderplugin::first; p; p = p->next) {
		if(p->translate(id))
			return true;
	}
	return false;
}