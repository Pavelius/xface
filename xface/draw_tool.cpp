#include "draw.h"

bool draw::tool(const rect& rc, bool disabled, bool checked, bool press, bool focused, int key) {
	if(disabled)
		return false;
	bool result = false;
	rect rch = rc; rch.x2--; rch.y2--;
	auto hilited = ishilite(rch);
	auto pressed = false;
	if(hilited && hot.key == MouseLeft && hot.pressed == press) {
		pressed = true;
		result = true;
	}
	if(focused && key && hot.key == key)
		result = true;
	if(checked)
		pressed = true;
	if(hilited) {
		rectf(rc, colors::edit.mix(colors::window, 128));
		rectb(rc, colors::border);
	} else if(pressed) {
		rectf(rc, colors::edit);
		rectb(rc, colors::border);
	}
	return result;
}