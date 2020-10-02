#include "crt.h"
#include "draw_control.h"

using namespace draw;

static anyval cmd_value;

static void setvar() {
	cmd_value.set(cmd_value.getvalue());
}

static void xorvar() {
	auto m = cmd_value.get();
	auto v = cmd_value.getvalue();
	if(!v)
		v = 1;
	m ^= v;
	cmd_value.set(m);
}

void draw::setposition(int& x, int& y, int& width, int padding) {
	if(padding == -1) {
		x += metrics::edit.x1;
		y += metrics::edit.y1;
		width += metrics::edit.x2 - metrics::edit.x1;
	} else {
		x += padding;
		y += padding;
		width -= padding * 2;
	}
}

void draw::titletext(int& x, int y, int& width, unsigned flags, const char* label, int title, const char* separator) {
	if(!separator)
		separator = ":";
	char temp[1024];
	if(!title)
		title = 128;
	zcpy(temp, label, sizeof(temp) - 2);
	zcat(temp, separator);
	text(x, y + 4, temp);
	x += title;
	width -= title;
}

int	draw::button(int x, int y, int width, const anyval& value, bool& result, const char* label, const char* tips, int key) {
	setposition(x, y, width);
	struct rect rc = {x, y, x + width, y + 4 * 2 + draw::texth()};
	auto focus = isfocused(rc, value);
	if(buttonh({x, y, x + width, rc.y2},
		false, focus, false, true, label, key, false, tips)
		|| (focus && hot.key == KeyEnter)) {
		result = true;
	}
	if(label && label[0] && ishilite(rc))
		tooltips(tips);
	return rc.height() + metrics::padding * 2;
}

bool draw::buttonr(int& x, int y, fnevent proc, const char* label, void* object, int key) {
	auto width = textw(label) + metrics::padding * 2 + 4 * 2;
	if(width < 100)
		width = 100;
	x -= width;
	auto x1 = x;
	setposition(x1, y, width);
	struct rect rc = {x1, y, x1 + width, y + 4 * 2 + draw::texth()};
	const anyval av(proc, 0, 0);
	auto focus = isfocused(rc, av);
	auto result = false;
	if(buttonh({x1, y, x1 + width, rc.y2},
		false, focus, false, true, label, key, false, 0)
		|| (focus && hot.key == KeyEnter)) {
		execute(proc,0,0,object);
	}
	return result;
}

int	draw::button(int x, int y, int width, fnevent proc, const char* label, const char* tips, int key) {
	auto result = false;
	auto dy = button(x, y, width, anyval((void*)proc, 0, 0), result, label, tips, key);
	if(result)
		execute(proc);
	return dy;
}

int draw::radio(int x, int y, int width, const anyval& av, const char* label, const char* tips) {
	draw::state push;
	setposition(x, y, width, 1);
	rect rc = {x, y, x + width, y};
	rect rc1 = {rc.x1 + 22, rc.y1, rc.x2, rc.y2};
	draw::textw(rc1, label);
	rc1.offset(-2);
	rc.y1 = rc1.y1;
	rc.y2 = rc1.y2;
	rc.x2 = rc1.x2;
	unsigned flags = 0;
	if(isfocused(rc, av))
		flags |= Focused;
	if(av.get() == av.getvalue())
		flags |= Checked;
	clipart(x + 2, y + imax((rc1.height() - 14) / 2, 0), width, flags, ":radio");
	bool need_select = false;
	auto a = ishilite(rc);
	if(a && !isdisabled(flags) && hot.key == MouseLeft) {
		if(!hot.pressed)
			need_select = true;
	}
	if(isfocused(flags)) {
		rectx({rc1.x1, rc1.y1, rc1.x2, rc1.y2}, draw::fore);
		if(!isdisabled(flags) && hot.key == KeySpace)
			need_select = true;
	}
	if(need_select) {
		cmd_value = av;
		execute(setvar);
	}
	rc = rc1; rc.offset(2);
	draw::text(rc, label);
	if(tips && a && !hot.pressed)
		tooltips(tips);
	return rc1.height() + 2;
}

int draw::checkbox(int x, int y, int width, const anyval& value, const char* label, const char* tips) {
	draw::state push;
	setposition(x, y, width, 1);
	rect rc = {x, y, x + width, y};
	rect rc1 = {rc.x1 + 22, rc.y1, rc.x2, rc.y2};
	draw::textw(rc1, label);
	rc.y1 = rc1.y1;
	rc.y2 = rc1.y2;
	rc.x2 = rc1.x2;
	unsigned flags = 0;
	if(isfocused(rc, value))
		flags |= Focused;
	auto v1 = value.getvalue();
	if(!v1)
		v1 = 1;
	if((value.get() & v1) != 0)
		flags |= Checked;
	clipart(x + 2, y + imax((rc1.height() - 14) / 2, 0), 0, flags, ":check");
	decortext(flags);
	auto a = draw::ishilite(rc);
	auto need_value = false;
	if(a && !isdisabled(flags) && hot.key == MouseLeft) {
		if(!hot.pressed)
			need_value = true;
	}
	if(isfocused(flags)) {
		draw::rectx({rc1.x1 - 2, rc1.y1 - 1, rc1.x2 + 2, rc1.y2 + 1}, draw::fore);
		if(hot.key == KeySpace)
			need_value = true;
	}
	if(need_value) {
		cmd_value = value;
		execute(xorvar);
	}
	draw::text(rc1, label);
	if(tips && a && !hot.pressed)
		tooltips(tips);
	return rc1.height() + 2;
}