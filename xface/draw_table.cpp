#include "crt.h"
#include "draw_control.h"

using namespace draw::controls;

void table::update_columns() {
	int w1 = view_rect.width() - 1;
	int w2 = 0;
	int c2 = 0;
	const int min_width = 8;
	if(!columns)
		return;
	for(auto& e : columns) {
		if(e.size == SizeAuto) {
			w2 += min_width;
			c2++;
		} else {
			if(e.method->minimal_width) {
				if(e.width < e.method->minimal_width)
					e.width = e.method->minimal_width;
			}
			w2 += e.width;
		}
	}
	// calculate auto sized width
	if(w2 < w1 && c2) {
		int d1 = w1 - w2;
		int d2 = d1 / c2;
		for(auto& e : columns) {
			if(e.size == SizeAuto) {
				if(d2 < d1)
					e.width = min_width + d2;
				else {
					e.width = min_width + d1;
					break;
				}
			}
		}
	}
	current_column = getvalid(current_column, 1);
}

int	table::getvalid(int column, int direction) const {
	unsigned count = 0;
	while(true) {
		if(count >= columns.count)
			return current_column;
		if(column < 0 || (unsigned)column >= columns.count)
			return current_column;
		if(!columns[column].isvisible()
			|| columns[column].size == SizeInner) {
			if(direction)
				column += direction;
			else
				column += 1;
			count++;
			continue;
		}
		break;
	}
	return column;
}

void table::select(int index, int column) {
	current_column = column;
	list::select(index, column);
}

void table::mouseselect(int id, bool pressed) {
	if(current_hilite == -1 || current_hilite_column == -1)
		return;
	if(pressed)
		select(current_hilite, getvalid(current_hilite_column));
}

int table::rowheader(const rect& rc) const {
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
	color active = colors::button.mix(colors::edit, 128);
	color a1 = active.lighten();
	color a2 = active.darken();
	rect r1;
	r1.x1 = rch.x1 - origin_width;
	r1.x2 = r1.x1;
	r1.y1 = rch.y1;
	r1.y2 = rch.y2;
	for(unsigned i = 0; i < columns.count; i++) {
		if(!columns[i].isvisible())
			continue;
		r1.x2 = r1.x2 + columns[i].width;
		if(columns[i].size == SizeInner)
			continue;
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
		// Нарисуем границу только когда она далеко от края
		// Чтобы она была не видна, если ширина элемента впритык к краю.
		if(r1.x2 < rc.x2 - 1)
			line(r1.x2, r1.y1, r1.x2, r1.y2, colors::border);
		temp[0] = 0;
		auto p = getheader(temp, temp + sizeof(temp) / sizeof(temp[0]) - 1, i);
		if(p)
			textc(r1.x1 + header_padding, r1.y1 + header_padding, r1.width() - header_padding * 2, p);
		a = area({r1.x2 - header_padding, r1.y1, r1.x2, r1.y2});
		if(a == AreaHilited || a == AreaHilitedPressed) {
			hot.cursor = CursorLeftRight;
			if(hot.pressed && hot.key == MouseLeft) {
				drag::begin((int)this, DragColumn);
				drag::mouse.x = hot.mouse.x - r1.x1;
				drag::value = i;
			}
		}
		r1.x1 = r1.x2;
	}
	return height;
}

void table::row(const rect& rc, int index) const {
	char temp[260];
	area(rc);
	if(select_full_row)
		rowhilite(rc, index);
	auto current_column = getcolumn();
	auto x1 = rc.x1;
	for(unsigned i = 0; i < columns.count; i++) {
		auto pc = columns.data + i;
		if(!pc->isvisible())
			continue;
		rect rt = {x1 + 4, rc.y1 + 4, x1 + pc->width - 4, rc.y2 - 4};
		if(show_grid_lines && columns[i].size != SizeInner) {
			if(rt.x2 + 3 < rc.x2 - 1)
				draw::line(rt.x2 + 3, rt.y1 - 4, rt.x2 + 3, rt.y2 + 3, colors::border);
		}
		area(rt);
		if(!select_full_row) {
			if(index == current && i == current_column)
				hilight({rt.x1 - 4, rt.y1 - 4, rt.x2 + 3, rt.y2 + 3});
		}
		temp[0] = 0;
		(this->*pc->method->render)(rt, index, i);
		x1 += pc->width;
	}
}

void table::viewtotal(rect rc) const {
	rc.offset(1, 1);
	rc.offset(4, 4);
	for(unsigned i = 0; i < columns.count; i++) {
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
			draw::text(rt, p, AlignRight);
		rc.x1 += columns[i].width;
	}
}

void table::view(const rect& rc) {
	view_rect = rc;
	current_hilite_column = -1;
	update_columns();
	rect rt;
	rt.y1 = rc.y1;
	rt.y2 = rc.y2;
	maximum_width = 0;
	for(unsigned i = 0; i < columns.count; i++) {
		if(!columns[i].isvisible())
			continue;
		rt.x1 = rc.x1 - origin_width + maximum_width;
		rt.x2 = rt.x1 + columns[i].width;
		if(hot.mouse.in(rt))
			current_hilite_column = i;
		maximum_width += columns[i].width;
		current_column_maximum = i;
	}
	rt = rc;
	if(show_header)
		rt.y1 += rowheader(rt);
	if(show_totals) {
		list::view({rt.x1, rt.y1, rt.x2, rt.y2 - getrowheight()});
		viewtotal({rt.x1, rt.y2 - getrowheight(), rt.x2, rt.y2});
	} else
		list::view(rt);
}

bool table::keyinput(unsigned id) {
	switch(id) {
	case KeyLeft:
		current_column = getvalid(current_column - 1, -1);
		break;
	case KeyRight:
		current_column = getvalid(current_column + 1);
		break;
	default: return list::keyinput(id);
	}
	return true;
}

void table::redraw() {
	view(view_rect);
}

column* table::addcol(const char* id, const char* name, const char* type, draw::column_size_s size, int width) {
	auto pf = getvisuals()->find(type);
	if(!pf)
		return 0;
	auto p = columns.add();
	p->method = pf;
	p->id = szdup(id);
	p->title = szdup(name);
	p->tips = 0;
	p->size = size;
	if(p->size == SizeDefault)
		p->size = p->method->size;
	p->width = width;
	if(!p->width)
		p->width = p->method->default_width;
	if(!p->width)
		p->width = 100;
	return p;
}

bool table::changefield(const rect& rc, unsigned flags, char* result, const char* result_maximum) {
	auto push_focus = getfocus();
	auto pn = getname(result, result_maximum, current, current_column);
	if(pn != result)
		zcpy(result, pn, result_maximum - result - 1);
	textedit te(result, result_maximum - result - 1, true);
	setfocus((int)&te, true);
	te.show_border = false;
	te.align = flags;
	te.rctext.x2++;
	auto r = te.editing({current_rect.x1, current_rect.y1, current_rect.x2, current_rect.y2});
	setfocus(push_focus, true);
	return r;
}

void table::changenumber(const rect& rc, int line, int column) {
	char temp[32];
	if(changefield(rc, AlignRight, temp, zendof(temp)))
		changing(current, current_column, temp);
}

void table::changetext(const rect& rc, int line, int column) {
	char temp[8192];
	if(changefield(rc, 0, temp, zendof(temp)))
		changing(current, current_column, temp);
}

void table::changecheck(const rect& rc, int line, int column) {
	if(getnumber(current, current_column))
		changing(current, current_column, "1");
	else
		changing(current, current_column, "0");
}

bool table::change(bool run) {
	if(read_only)
		return false;
	if(!columns)
		return false;
	if(zchr(columns[current_column].id, '.'))
		return false;
	auto pv = columns[current_column].method;
	if(!pv)
		return false;
	if(!pv->change)
		return false;
	if(run) {
		if(!current_rect)
			return true;
		(this->*pv->change)(current_rect, current, current_column);
	}
	return true;
}

void table::checkbox(const rect& rc, int line, int column) const {
	auto number_value = getnumber(line, column);
	clipart(rc.x1 + 2, rc.y1 + imax((rc.height() - 14) / 2, 0), 0, number_value ? Check : 0, ":check");
}

void table::fieldtext(const rect& rc, int line, int column) const {
	char temp[260];
	auto p = getname(temp, temp + sizeof(temp) / sizeof(temp[0]) - 1, line, column);
	if(p)
		draw::text(rc, p, AlignLeft);
}

void table::fieldimage(const rect& rc, int line, int column) const {
	auto v = getnumber(line, column);
	auto s = gettreeimages();
	if(s)
		image(rc.x1 + rc.width() / 2, rc.y1 + rc.height() / 2, s, v, 0);
}

void table::fieldnumber(const rect& rc, int line, int column) const {
	char temp[32];
	auto v = getnumber(line, column);
	szprints(temp, zendof(temp), "%1i", v);
	draw::text(rc, temp, AlignRight);
}

void table::fieldpercent(const rect& rc, int line, int column) const {
	char temp[32];
	auto v = getnumber(line, column);
	szprints(temp, zendof(temp), "%1i%%", v);
	draw::text(rc, temp, AlignRight);
}

const visual* table::getvisuals() const {
	static visual elements[] = {{"checkbox", "Пометка", 20, 20, SizeFixed, &table::checkbox, &table::changecheck},
	{"text", "Текстовое поле", 8, 200, SizeResized, &table::fieldtext, &table::changetext},
	{"number", "Числовое поле", 8, 80, SizeResized, &table::fieldnumber, &table::changenumber},
	{"percent", "Процент", 40, 60, SizeResized, &table::fieldpercent, &table::changenumber},
	{"image", "Изображение", 20, 20, SizeInner, &table::fieldimage},
	{}};
	return elements;
}