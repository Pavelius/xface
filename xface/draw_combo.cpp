#include "bsdata.h"
#include "crt.h"
#include "draw.h"
#include "draw_control.h"

using namespace draw;

//static void* find_lesser(const bsval& e) {
//	auto b = bsdata::find(e.type);
//	if(!b)
//		return 0;
//	auto n2 = e.getname();
//	auto pe = b->end();
//	auto result = 0;
//	void* result_data = 0;
//	for(auto pb = b->begin(); pb < pe; pb += b->size) {
//		bsval e1;
//		e1.data = pb;
//		e1.type = e.type;
//		auto n1 = e1.getname();
//		auto i1 = strcmp(n1, n2);
//		if(result > i1) {
//			result = i1;
//			result_data = e1.data;
//		}
//	}
//	return result_data;
//}

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