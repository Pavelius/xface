#include "crt.h"
#include "draw_control.h"

using namespace draw::controls;

void table::select(int index, int column) {
	current_column = column;
	list::select(index, column);
}

void table::mouseselect(int id, bool pressed) {
	if(current_hilite == -1 || current_hilite_column == -1)
		return;
	if(pressed)
		select(current_hilite, current_hilite_column);
}

int table::rowheader(rect rc) const {
	const int header_padding = 4;
	char temp[260]; auto height = getrowheight();
	rect rch = {rc.x1, rc.y1, rc.x2, rc.y1 + height};
	color b1 = colors::button.lighten();
	color b2 = colors::button.darken();
	if(no_change_order)
		b1 = b1.mix(b2, 192);
	gradv(rch, b1, b2);
	rectb(rch, colors::border);
	draw::state push;
	draw::setclip(rc);
	for(auto i = 0; columns[i]; i++) {
		if(!columns[i].isvisible())
			continue;
		rect r1 = {rc.x1, rc.y1, rc.x1 + columns[i].width, rc.y1 + height};
		color active = colors::button.mix(colors::edit, 128);
		color a1 = active.lighten();
		color a2 = active.darken();
		auto a = area(r1);
		if(a == AreaHilited) {
			if(columns[i].tips && columns[i].tips[0])
				tooltips(columns[i].tips);
		}
		if(!no_change_order) {
			switch(a) {
			case AreaHilited:
				gradv({r1.x1 + 1, r1.y1 + 1, r1.x2, r1.y2}, a1, a2);
				break;
			case AreaHilitedPressed:
				gradv({r1.x1 + 1, r1.y1 + 1, r1.x2, r1.y2}, a2, a1);
				break;
			}
			if((a == AreaHilited || a == AreaHilitedPressed) && hot.key == MouseLeft && !hot.pressed)
				clickcolumn(i);
		}
		line(r1.x2, r1.y1, r1.x2, r1.y2, colors::border);
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
	area(rc);
	if(select_full_row)
		rowhilite(rc, index);
	rc.offset(4, 4);
	auto current_column = getcolumn();
	for(auto i = 0; columns[i]; i++) {
		if(!columns[i].isvisible())
			continue;
		rect rt = {rc.x1, rc.y1, rc.x1 + columns[i].width - 4, rc.y2};
		if(show_grid_lines)
			draw::line(rt.x2 - 1, rt.y1 - 4, rt.x2 - 1, rt.y2 + 4, colors::border);
		area(rt);
		if(!select_full_row) {
			if(index == current && i == current_column)
				hilight({rt.x1 - 4, rt.y1 - 4, rt.x2 - 1, rt.y2 + 3});
		}
		temp[0] = 0;
		const char* p;
		int number_value;
		switch(columns[i].getcontol()) {
		case Field:
			p = getname(temp, temp + sizeof(temp) / sizeof(temp[0]) - 1, index, i);
			if(p)
				draw::text(rt, p, (columns[i].flags & AlignMask));
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
	for(auto i = 0; columns[i]; i++) {
		if(!columns[i].isvisible())
			continue;
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
	current_hilite_column = -1;
	maximum_width = 0;
	for(auto i = 0; columns[i]; i++) {
		if(!columns[i].isvisible())
			continue;
		rect rt;
		rt.x1 = rc.x1 - origin_width + maximum_width;
		rt.x2 = rt.x1 + columns[i].width;
		rt.y1 = rc.y1;
		rt.y2 = rc.y2;
		if(hot.mouse.in(rt))
			current_hilite_column = i;
		maximum_width += columns[i].width;
		current_column_maximum = i;
	}
	if(show_header)
		rc.y1 += rowheader(rc);
	if(show_totals) {
		list::view({rc.x1, rc.y1, rc.x2, rc.y2 - getrowheight()});
		viewtotal({rc.x1, rc.y2 - getrowheight(), rc.x2, rc.y2});
	} else
		list::view(rc);
}

void table::keyleft(int id) {
	if(current_column > 0)
		current_column--;
}

void table::keyright(int id) {
	if(current_column < current_column_maximum)
		current_column++;
}
