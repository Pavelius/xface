#include "crt.h"
#include "draw_control.h"

using namespace draw::controls;

void table::update_columns(const rect& rc) {
	int w1 = rc.width();
	int w2 = 0;
	int c2 = 0;
	const int min_width = 8;
	if(!columns)
		return;
	// Get count of al elements
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
	// Calculate auto sized width
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

void table::row(const rect& rc, int index) {
	char temp[260];
	area(rc);
	if(select_mode == SelectRow)
		rowhilite(rc, index);
	auto current_column = getcolumn();
	auto x1 = rc.x1;
	auto need_tree = true;
	auto level_ident = 0;
	for(unsigned i = 0; i < columns.count; i++) {
		auto pc = columns.data + i;
		if(!pc->isvisible())
			continue;
		if(need_tree) {
			auto level = getlevel(index);
			if(level) {
				int dy = getident();
				level_ident = level * dy;
				x1 = x1 + level_ident;
				treemark({x1 - dy, rc.y1, x1, rc.y2}, index, level);
			}
			need_tree = false;
		}
		rect rt = {x1 + 4, rc.y1 + 4, x1 + pc->width - 4, rc.y2 - 4};
		if(level_ident) {
			if(columns[i].size != SizeInner && columns[i].size != SizeFixed) {
				auto mx = rt.width();
				if(mx > level_ident)
					mx = level_ident;
				rt.x2 -= mx;
				level_ident -= mx;
			}
		}
		if(show_grid_lines && columns[i].size != SizeInner)
			draw::line(rt.x2 + 3, rt.y1 - 4, rt.x2 + 3, rt.y2 + 3, colors::border);
		area(rt);
		temp[0] = 0;
		(this->*pc->method->render)(rt, index, i);
		x1 += rt.width() + 8;
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

void table::ensurevisible() {
	list::ensurevisible();
	auto rc = getrect(current, current_column);
	auto x1 = rc.x1 - (view_rect.x1 - origin_width);
	auto x2 = rc.x2 - (view_rect.x1 - origin_width);
	if(x1 > origin_width + view_rect.width())
		origin_width = x1;
	if(x2 < origin_width)
		origin_width = x1;
}

rect table::getrect(int row, int column) const {
	rect rs;
	rs.x1 = view_rect.x1 - origin_width;
	rs.x2 = rs.x1;
	rs.y1 = view_rect.y1 - origin + lines_per_page * row;
	rs.y2 = rs.y1 + getrowheight();
	for(auto i = 0; i < column; i++) {
		if(!columns[i].isvisible())
			continue;
		rs.x1 = rs.x2;
		rs.x2 = rs.x1 + columns[i].width;
	}
	return rs;
}

void table::view(const rect& rc) {
	current_hilite_column = -1;
	rect rt;
	rt.y1 = rc.y1;
	rt.y2 = rc.y2;
	update_columns(rc);
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
	// Для того, чтобы не было видно самую правую границу колонки
	if(maximum_width > 0)
		maximum_width -= 1;
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
	current_column = getvalid(current_column, 1);
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

void table::cellbox(const rect& rc, int line, int column) {
	auto number_value = getnumber(line, column);
	clipart(rc.x1 + 2, rc.y1 + imax((rc.height() - 14) / 2, 0), 0, number_value ? Check : 0, ":check");
}

void table::cellhilite(const rect& rc, int line, int column, const char* text, image_flag_s aling) const {
	if(line == current && column == current_column) {
		rect rch = {rc.x1 - 4, rc.y1 - 4, rc.x2 + 3, rc.y2 + 3};
		switch(select_mode) {
		case SelectCell:
			hilight(rch);
			break;
		case SelectText:
			switch(aling) {
			case AlignRight:
			case AlignRightCenter:
			case AlignRightBottom:
				hilight({rc.x2 - 3 - draw::textw(text), rc.y1 - 4, rc.x2 + 3, rc.y2 + 3}, &rch);
				break;
			default:
				hilight({rc.x1 - 4, rc.y1 - 4, rc.x1 + draw::textw(text) + 5, rc.y2 + 3}, &rch);
				break;
			}
			break;
		}
	}
}

void table::celltext(const rect& rc, int line, int column) {
	char temp[260];
	auto p = getname(temp, temp + sizeof(temp) / sizeof(temp[0]) - 1, line, column);
	if(p) {
		cellhilite(rc, line, column, p, AlignLeft);
		draw::text(rc, p, AlignLeft);
	}
}

void table::cellimage(const rect& rc, int line, int column) {
	auto v = getnumber(line, column);
	auto s = gettreeimages();
	if(s)
		image(rc.x1 + rc.width() / 2, rc.y1 + rc.height() / 2, s, v, 0);
}

void table::cellnumber(const rect& rc, int line, int column) {
	char temp[32];
	auto v = getnumber(line, column);
	szprints(temp, zendof(temp), "%1i", v);
	cellhilite(rc, line, column, temp, AlignRight);
	draw::text(rc, temp, AlignRight);
}

void table::cellpercent(const rect& rc, int line, int column) {
	char temp[32];
	auto v = getnumber(line, column);
	szprints(temp, zendof(temp), "%1i%%", v);
	cellhilite(rc, line, column, temp, AlignRight);
	draw::text(rc, temp, AlignRight);
}

const visual* table::getvisuals() const {
	static visual elements[] = {{"checkbox", "Пометка", 20, 20, SizeFixed, &table::cellbox, &table::changecheck},
	{"text", "Текстовое поле", 8, 200, SizeResized, &table::celltext, &table::changetext},
	{"number", "Числовое поле", 8, 80, SizeResized, &table::cellnumber, &table::changenumber},
	{"percent", "Процент", 40, 60, SizeResized, &table::cellpercent, &table::changenumber},
	{"image", "Изображение", 20, 20, SizeInner, &table::cellimage},
	{}};
	return elements;
}