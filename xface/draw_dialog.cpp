#include "crt.h"
#include "draw_control.h"

using namespace draw;

void draw::setposition(int& x, int& y, int& width, int padding) {
	if(padding==-1)
		padding = metrics::padding;
	x += padding;
	y += padding;
	width -= padding * 2;
}

void draw::focusing(int id, unsigned& flags, rect rc) {
	if(flags&Disabled)
		return;
	addelement(id, rc);
	if(!getfocus())
		setfocus(id, true);
	if(getfocus() == id)
		flags |= Focused;
	else if(area(rc) == AreaHilitedPressed && hot.key == MouseLeft && hot.pressed) {
		setfocus(id, false);
		hot.key = MouseLeft;
	}
}

void draw::titletext(int& x, int y, int& width, unsigned flags, const char* label, int title) {
	char temp[1024];
	if(!title)
		title = 128;
	zcpy(temp, label, sizeof(temp) - 2);
	zcat(temp, ":");
	text(x, y + 4, temp);
	x += title;
	width -= title;
}

int	draw::button(int x, int y, int width, unsigned flags, const cmd& ev, const char* label, const char* tips, int key) {
	setposition(x, y, width);
	struct rect rc = {x, y, x + width, y + 4 * 2 + draw::texth()};
	focusing(ev.getid(), flags, rc);
	if(buttonh({x, y, x + width, rc.y2},
		ischecked(flags), isfocused(flags), isdisabled(flags), true, label, key, false, tips)
		|| (isfocused(flags) && hot.key == KeyEnter)) {
		ev.execute();
	}
	if(label && label[0] && areb(rc))
		tooltips(tips);
	return rc.height() + metrics::padding * 2;
}

int draw::radio(int x, int y, int width, unsigned flags, const cmd& ev, const char* label, const char* tips) {
	draw::state push;
	setposition(x, y, width, 1);
	rect rc = {x, y, x + width, y};
	rect rc1 = {rc.x1 + 22, rc.y1, rc.x2, rc.y2};
	draw::textw(rc1, label);
	rc1.offset(-2);
	rc.y1 = rc1.y1;
	rc.y2 = rc1.y2;
	rc.x2 = rc1.x2;
	decortext(flags);
	focusing(ev.getid(), flags, rc);
	clipart(x + 2, y + imax((rc1.height() - 14) / 2, 0), width, flags, ":radio");
	bool need_select = false;
	auto a = draw::area(rc);
	if((a == AreaHilited || a == AreaHilitedPressed) && !isdisabled(flags) && hot.key == MouseLeft) {
		if(!hot.pressed)
			need_select = true;
	}
	if(isfocused(flags)) {
		draw::rectx({rc1.x1, rc1.y1, rc1.x2, rc1.y2}, draw::fore);
		if(!isdisabled(flags) && hot.key == KeySpace)
			need_select = true;
	}
	if(need_select)
		ev.execute();
	rc = rc1; rc.offset(2);
	draw::text(rc, label);
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc1.height() + 2;
}

int draw::checkbox(int x, int y, int width, unsigned flags, const cmd& ev, const char* label, const char* tips) {
	draw::state push;
	setposition(x, y, width, 1);
	rect rc = {x, y, x + width, y};
	rect rc1 = {rc.x1 + 22, rc.y1, rc.x2, rc.y2};
	draw::textw(rc1, label);
	rc.y1 = rc1.y1;
	rc.y2 = rc1.y2;
	rc.x2 = rc1.x2;
	focusing(ev.getid(), flags, rc);
	clipart(x + 2, y + imax((rc1.height() - 14) / 2, 0), 0, flags, ":check");
	decortext(flags);
	auto a = draw::area(rc);
	auto need_value = false;
	if((a == AreaHilited || a == AreaHilitedPressed) && !isdisabled(flags) && hot.key == MouseLeft) {
		if(!hot.pressed)
			need_value = true;
	}
	if(isfocused(flags)) {
		draw::rectx({rc1.x1 - 2, rc1.y1 - 1, rc1.x2 + 2, rc1.y2 + 1}, draw::fore);
		if(hot.key == KeySpace)
			need_value = true;
	}
	if(need_value)
		ev.execute();
	draw::text(rc1, label);
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc1.height() + 2;
}

static void change_checkbox() {
	auto pv = (bool*)hot.param;
	*pv = !*pv;
}

int	draw::checkbox(int x, int y, int width, bool& value, const char* label, const char* tips) {
	unsigned flags = value ? Checked : 0;
	return checkbox(x, y, width, flags, cmd(change_checkbox, (int)&value), label, tips);
}