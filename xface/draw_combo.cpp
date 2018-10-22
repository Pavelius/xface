#include "bsreq.h"
#include "crt.h"
#include "draw.h"
#include "draw_control.h"

using namespace draw;

int	draw::combobox(int x, int y, int width, unsigned flags, const bsval& cmd, const char* header_label, const char* tips, int header_width) {
	draw::state push;
	setposition(x, y, width);
	decortext(flags);
	if(header_label && header_label[0])
		titletext(x, y, width, flags, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	focusing((int)cmd.type->ptr(cmd.data), flags, rc);
	auto focused = isfocused(flags);
	auto result = false;
	auto a = area(rc);
	if(isdisabled(flags)) {
		gradv(rc, colors::button.lighten(), colors::button.darken());
		rectb(rc, colors::border.mix(colors::window));
	} else {
		if((a == AreaHilited || a == AreaHilitedPressed) && hot.key == MouseLeft && !hot.pressed)
			result = true;
		color active = colors::button.mix(colors::edit, 128);
		switch(a) {
		case AreaHilited: gradv(rc, active.lighten(), active.darken()); break;
		case AreaHilitedPressed: gradv(rc, active.darken(), active.lighten()); break;
		default: gradv(rc, colors::button.lighten(), colors::button.darken()); break;
		}
		rectb(rc, focused ? colors::active : colors::border);
	}
	rect rco = rc; rco.offset(4, 4);
	auto name = cmd.getname();
	if(name && name[0])
		textc(rco.x1, rco.y1, rco.width(), name);
	if(tips && a == AreaHilited)
		tooltips(tips);
	return rc.height() + metrics::padding * 2;
}