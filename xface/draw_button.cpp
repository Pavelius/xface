#include "draw.h"

bool draw::buttonv(rect rc, bool checked, bool focused, bool disabled, bool border, const char* string, int key, bool press) {
	bool result = false;
	if(disabled) {
		gradh(rc, colors::button.lighten(), colors::button.darken());
		rectf(rc, colors::border.mix(colors::window));
	} else {
		if(focused && key && hot::key == key)
			result = true;
		areas a = area(rc);
		if((a == AreaHilited || a == AreaHilitedPressed) && hot::key == MouseLeft && hot::pressed == press)
			result = true;
		if(checked)
			a = AreaHilitedPressed;
		color active = colors::button.mix(colors::edit, 128);
		switch(a) {
		case AreaHilited:
			gradh(rc, active.lighten(), active.darken());
			break;
		case AreaHilitedPressed:
			gradh(rc, active.darken(), active.lighten());
			break;
		default:
			gradh(rc, colors::button.lighten(), colors::button.darken());
			break;
		}
		if(border)
			rectb(rc, focused ? colors::active : colors::border);
	}
	if(string)
		text(rc, string, AlignCenter);
	return result;
}

bool draw::buttonh(rect rc, bool checked, bool focused, bool disabled, bool border, color value, const char* string, int key, bool press, const char* tips) {
	draw::state push;
	bool result = false;
	struct rect rcb = {rc.x1 + 1, rc.y1 + 1, rc.x2, rc.y2};
	areas a = area(rc);
	//fore = (value.gray().r > 32) ? colors::white : colors::black;
	if(disabled) {
		gradv(rcb, value.lighten(), value.darken());
		if(border)
			rectb(rc, colors::border.mix(colors::window));
		fore = fore.mix(value, 64);
	} else {
		if(focused && key && hot::key == key)
			result = true;
		if((a == AreaHilited || a == AreaHilitedPressed) && hot::key == MouseLeft && hot::pressed == press)
			result = true;
		if(checked)
			a = AreaHilitedPressed;
		color active = value.mix(colors::edit, 128);
		color a1 = active.lighten();
		color a2 = active.darken();
		color b1 = value.lighten();
		color b2 = value.darken();
		switch(a) {
		case AreaHilited:
			gradv(rcb, a1, a2);
			break;
		case AreaHilitedPressed:
			gradv(rcb, a2, a1);
			break;
		default:
			gradv(rcb, b1, b2);
			break;
		}
		if(border)
			rectb(rc, focused ? colors::active : colors::border);
	}
	if(string)
		text(rc, string, AlignCenterCenter);
	if(tips && a == AreaHilited) {
		char temp[32];
		if(key)
			tooltips(rc.x1, rc.y1, rc.width(), "%1 (%2)", tips, key2str(temp, key));
		else
			tooltips(rc.x1, rc.y1, rc.width(), tips);
	}
	return result;
}

bool draw::buttonh(rect rc, bool checked, bool focused, bool disabled, bool border, const char* string, int key, bool press, const char* tips) {
	return buttonh(rc, checked, focused, disabled, border, colors::button, string, key, press, tips);
}