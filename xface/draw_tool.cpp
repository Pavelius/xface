#include "draw.h"

bool draw::tool(const rect& rc, bool disabled, bool checked, bool press, bool focused, int key) {
	if(disabled)
		return false;
	bool result = false;
	rect rch = rc; rch.x2--; rch.y2--;
	areas a = area(rch);
	if((a == AreaHilited || a == AreaHilitedPressed) && hot.key == MouseLeft && hot.pressed == press)
		result = true;
	if(focused && key && hot.key == key)
		result = true;
	if(checked)
		a = AreaHilitedPressed;
	switch(a) {
	case AreaHilited:
		rectf(rc, colors::edit.mix(colors::window, 128));
		rectb(rc, colors::border);
		break;
	case AreaHilitedPressed:
		rectf(rc, colors::edit);
		rectb(rc, colors::border);
		break;
	default:
		break;
	}
	return result;
}