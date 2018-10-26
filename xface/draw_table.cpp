#include "crt.h"
#include "draw_control.h"

using namespace draw::controls;

int table::rowheader(rect rc) const {
	char temp[260]; auto height = getrowheight();
	rect rch = {rc.x1, rc.y1, rc.x2, rc.y1 + height};
	gradv(rch, colors::button.lighten(), colors::button.darken());
	rectb(rch, colors::border);
	for(auto i = 0; columns[i]; i++) {
		const int header_padding = 4;
		rect r1 = {rc.x1, rc.y1, rc.x1 + columns[i].width, rc.y1 + height};
		color active = colors::button.mix(colors::edit, 128);
		color a1 = active.lighten();
		color a2 = active.darken();
		color b1 = colors::button.lighten();
		color b2 = colors::button.darken();
		auto a = area(r1);
		switch(a) {
		case AreaHilited:
			gradv(r1, a1, a2);
			if(columns[i].tips && columns[i].tips[0])
				tooltips(columns[i].tips);
			break;
		case AreaHilitedPressed:
			gradv(r1, a2, a1);
			break;
		default:
			gradv(r1, b1, b2);
			break;
		}
		rectb(r1, colors::border);
		temp[0] = 0;
		auto p = getheader(temp, temp + sizeof(temp) / sizeof(temp[0]) - 1, i);
		if(p)
			textc(r1.x1 + header_padding, r1.y1 + header_padding, r1.width() - header_padding * 2, p);
		a = area({r1.x2 - header_padding, r1.y1, r1.x2, r1.y2});
		if(a == AreaHilited || a == AreaHilitedPressed) {
			hot.cursor = CursorLeftRight;
			if(hot.pressed && hot.key == MouseLeft) {
				//auto column_total_width = w - e.width;
				//drag::begin((int)this, DragColumn);
				//drag::mouse.x = hot.mouse.x - r1.x1;
				//drag::value = i;
			}
		}
		rc.x1 += columns[i].width;
	}
	return height;
}

static void table_check() {

}

void table::row(rect rc, int index) const {
	char temp[260];
	rowhilite(rc, index);
	rc.offset(4, 4);
	for(auto i = 0; columns[i]; i++) {
		rect rt = {rc.x1, rc.y1, rc.x1 + columns[i].width - 4, rc.y2};
		temp[0] = 0;
		const char* p;
		int number_value;
		switch(columns[i].getcontol()) {
		case Field:
			p = getname(temp, temp + sizeof(temp) / sizeof(temp[0]) - 1, index, i);
			if(p)
				draw::text(rt, p, (columns[i].flags & AlignMask));
			//number_value = getnumber(index, i);
			//if((columns[i].flags & HideZero) == 0 || number_value != 0) {
			//	szprints(temp, temp + sizeof(temp) + 1, "%1i", number_value);
			//	draw::text(rt, temp, columns[i].flags);
			//}
			break;
		case Check:
			number_value = getnumber(index, i);
			clipart(rt.x1 + 2, rt.y1 + imax((rt.height() - 14) / 2, 0), 0, number_value ? Check : 0, ":check");
			break;
		default:
			custom(temp, temp + sizeof(temp) - 1, rc, index, i);
			break;
		}
		rc.x1 += columns[i].width;
	}
}

void table::viewtotal(rect rc) const {
	rc.offset(1, 1);
	rc.offset(4, 4);
	for(auto i = 0; columns[i].title; i++) {
		char temp[260]; temp[0] = 0;
		rect rt = {rc.x1, rc.y1, rc.x1 + columns[i].width - 4, rc.y2};
		auto p = gettotal(temp, temp + sizeof(temp) - 1, i);
		if(!p) {
			auto result = gettotal(i);
			if(result) {
				szprints(temp, temp + sizeof(temp) - 1, "%1i", result);
				p = temp;
			}
		}
		if(p)
			draw::text(rt, p, columns[i].flags);
		rc.x1 += columns[i].width;
	}
}

void table::view(rect rc) {
	if(show_header)
		rc.y1 += rowheader(rc);
	if(show_totals) {
		list::view({rc.x1, rc.y1, rc.x2, rc.y2 - getrowheight()});
		viewtotal({rc.x1, rc.y2 - getrowheight(), rc.x2, rc.y2});
	} else
		list::view(rc);
}