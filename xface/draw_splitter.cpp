#include "draw.h"

static int split_value;

void draw::splitv(int x, int y, int& value, int height, int size, int minimum, int maximum, bool right_align) {
	rect rc;
	auto mode = false;
	if(dragactive(&value)) {
		value = split_value - (right_align ? (hot.mouse.x - dragmouse.x) : (dragmouse.x - hot.mouse.x));
		if(value < minimum)
			value = minimum;
		if(value > maximum)
			value = maximum;
		mode = true;
	}
	if(right_align)
		rc.x1 = x - value - size;
	else
		rc.x1 = x + value;
	rc.x2 = rc.x1 + size;
	rc.y1 = y;
	rc.y2 = rc.y1 + height;
	if(!mode)
		mode = ishilite(rc);
	if(mode && hot.key == MouseLeft && hot.pressed) {
		dragbegin(&value);
		split_value = value;
	}
	if(mode) {
		hot.cursor = cursor::LeftRight;
		if(hot.pressed)
			rectf(rc, colors::button.darken(), 128);
		else
			rectf(rc, colors::button, 128);
	}
}

void draw::splith(int x, int y, int width, int& value, int size, int minimum, int maximum, bool down_align) {
	struct rect rc;
	auto mode = false;
	if(dragactive(&value)) {
		value = split_value - (down_align ? (hot.mouse.y - dragmouse.y) : (dragmouse.y - hot.mouse.y));
		if(value < minimum)
			value = minimum;
		if(value > maximum)
			value = maximum;
		mode = true;
	}
	rc.x1 = x;
	rc.x2 = rc.x1 + width;
	if(down_align)
		rc.y1 = y - value - size;
	else
		rc.y1 = y + value;
	rc.y2 = rc.y1 + size;
	if(!mode)
		mode = ishilite(rc);
	if(mode && hot.key == MouseLeft && hot.pressed) {
		dragbegin(&value);
		split_value = value;
	}
	if(mode) {
		hot.cursor = cursor::UpDown;
		if(hot.pressed)
			rectf(rc, colors::button.darken(), 128);
		else
			rectf(rc, colors::button, 128);
	}
}