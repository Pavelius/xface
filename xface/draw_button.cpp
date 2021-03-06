#include "draw.h"

char* key2str(char* result, int key);

bool draw::addbutton(rect& rc, bool focused, const char* t1, unsigned k1, const char* tt1) {
	const int width = 18;
	rc.x2 -= width;
	auto result = draw::buttonh({rc.x2, rc.y1, rc.x2 + width, rc.y2},
		false, false, false, false,
		t1, k1, false, tt1);
	draw::line(rc.x2, rc.y1, rc.x2, rc.y2, colors::border);
	return result;
}

int draw::addbutton(rect& rc, bool focused, const char* t1, unsigned k1, const char* tt1, const char* t2, unsigned k2, const char* tt2) {
	const int width = 20;
	rc.x2 -= width;
	auto height = rc.height() / 2;
	auto result = 0;
	if(draw::buttonh({rc.x2, rc.y1, rc.x2 + width, rc.y1 + height},
		false, false, false, false,
		t1, k1, true, tt1))
		result = 1;
	if(draw::buttonh({rc.x2, rc.y1 + height, rc.x2 + width, rc.y1 + height*2},
		false, false, false, false,
		t2, k2, true, tt2))
		result = 2;
	if((hot.key == k2 || hot.key == k1) && !focused)
		result = 0;
	draw::line(rc.x2, rc.y1, rc.x2, rc.y2, colors::border);
	return result;
}

bool draw::buttonv(rect rc, bool checked, bool focused, bool disabled, bool border, const char* string, unsigned key, bool press) {
	bool result = false;
	if(disabled) {
		gradh(rc, colors::button.lighten(), colors::button.darken());
		rectf(rc, colors::border.mix(colors::window));
	} else {
		if(focused && key && hot.key == key)
			result = true;
		auto a = ishilite(rc);
		if(a && hot.key == MouseLeft && hot.pressed == press)
			result = true;
		if(checked)
			a = true;
		color active = colors::button.mix(colors::edit, 128);
		if(a) {
			if(hot.pressed)
				gradh(rc, active.darken(), active.lighten());
			else
				gradh(rc, active.lighten(), active.darken());
		} else
			gradh(rc, colors::button.lighten(), colors::button.darken());
		if(border)
			rectb(rc, focused ? colors::active : colors::border);
	}
	if(string)
		text(rc, string, AlignCenter);
	return result;
}

bool draw::buttonh(rect rc, bool checked, bool focused, bool disabled, bool border, color value, const char* string, unsigned key, bool press, const char* tips) {
	draw::state push;
	bool result = false;
	struct rect rcb = {rc.x1 + 1, rc.y1 + 1, rc.x2, rc.y2};
	auto a = ishilite(rcb);
	if(disabled) {
		gradv(rcb, value.lighten(), value.darken());
		if(border)
			rectb(rc, colors::border.mix(colors::window));
		fore = fore.mix(value, 64);
	} else {
		if(key && hot.key == key)
			result = true;
		if(a && hot.key == MouseLeft && hot.pressed == press)
			result = true;
		if(checked)
			a = true;
		color active = value.mix(colors::edit, 128);
		color a1 = active.lighten();
		color a2 = active.darken();
		color b1 = value.lighten();
		color b2 = value.darken();
		if(a) {
			if(hot.pressed)
				gradv(rcb, a2, a1);
			else
				gradv(rcb, a1, a2);
		} else
			gradv(rcb, b1, b2);
		if(border)
			rectb(rc, focused ? colors::active : colors::border);
		auto rco = rc; rco.offset(2, 2);
		if(focused)
			rectx(rco, colors::black);
	}
	if(string)
		text(rc, string, AlignCenterCenter);
	if(tips && a && !hot.pressed) {
		char temp[32];
		if(key)
			tooltips("%1 (%2)", tips, key2str(temp, key));
		else
			tooltips(tips);
	}
	return result;
}

bool draw::buttonh(rect rc, bool checked, bool focused, bool disabled, bool border, const char* string, unsigned key, bool press, const char* tips) {
	return buttonh(rc, checked, focused, disabled, border, colors::button, string, key, press, tips);
}