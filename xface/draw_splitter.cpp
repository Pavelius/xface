#include "draw.h"

static int split_value;

void draw::splitv(int x, int y, int& value, int height, int size, int minimum, int maximum, bool right_align) {
	rect rc;
	auto mode = AreaNormal;
	if(dragactive(&value)) {
		value = split_value - (right_align ? (hot.mouse.x - dragmouse.x) : (dragmouse.x - hot.mouse.x));
		mode = AreaHilitedPressed;
		if(value < minimum)
			value = minimum;
		if(value > maximum)
			value = maximum;
	}
	if(right_align)
		rc.x1 = x - value - size;
	else
		rc.x1 = x + value;
	rc.x2 = rc.x1 + size;
	rc.y1 = y;
	rc.y2 = rc.y1 + height;
	if(mode != AreaHilitedPressed)
		mode = area(rc);
	if(mode == AreaHilitedPressed && hot.key == MouseLeft && hot.pressed) {
		dragbegin(&value);
		split_value = value;
	}
	switch(mode) {
	case AreaHilited:
		hot.cursor = CursorLeftRight;
		rectf(rc, colors::button, 128);
		break;
	case AreaHilitedPressed:
		hot.cursor = CursorLeftRight;
		rectf(rc, colors::button.darken(), 128);
		break;
	default:
		break;
	}
}

void draw::splith(int x, int y, int width, int& value, int size, int minimum, int maximum, bool down_align) {
	struct rect rc;
	areas mode = AreaNormal;
	if(dragactive(&value)) {
		value = split_value - (down_align ? (hot.mouse.y - dragmouse.y) : (dragmouse.y - hot.mouse.y));
		mode = AreaHilitedPressed;
		if(value < minimum)
			value = minimum;
		if(value > maximum)
			value = maximum;
	}
	rc.x1 = x;
	rc.x2 = rc.x1 + width;
	if(down_align)
		rc.y1 = y - value - size;
	else
		rc.y1 = y + value;
	rc.y2 = rc.y1 + size;
	if(mode != AreaHilitedPressed)
		mode = area(rc);
	if(mode == AreaHilitedPressed && hot.key == MouseLeft && hot.pressed) {
		dragbegin(&value);
		split_value = value;
	}
	switch(mode) {
	case AreaHilited:
		hot.cursor = CursorUpDown;
		rectf(rc, colors::button, 128);
		break;
	case AreaHilitedPressed:
		hot.cursor = CursorUpDown;
		rectf(rc, colors::button.darken(), 128);
		break;
	default:
		break;
	}
}