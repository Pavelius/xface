#include "crt.h"
#include "datetime.h"
#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

static int		current_sort_column;
static bool		current_order;
static table*	current_element;

struct table_sort_param {
	table*		element;
	int			column;
};

int table::comparestr(int i1, int i2, void* param) {
	auto ps = (table_sort_param*)param;
	auto pc = ps->element;
	char t1[260], t2[260]; t1[0] = 0; t2[0] = 0;
	auto n1 = pc->columns[ps->column].gete(pc->get(i1), t1, t1 + sizeof(t1) - 1);
	auto n2 = pc->columns[ps->column].gete(pc->get(i2), t2, t2 + sizeof(t2) - 1);
	return strcmp(n1, n2);
}

int table::comparenum(int i1, int i2, void* param) {
	auto ps = (table_sort_param*)param;
	auto pe = ps->element;
	auto pc = &pe->columns[ps->column];
	return pc->get(pe->get(i1)) - pc->get(pe->get(i2));
}

void table::sort(int i1, int i2, bool ascending, proc_compare comparer, void* param) {
	if(ascending) {
		for(int i = i2; i > i1; i--) {
			for(int j = i1; j < i; j++)
				if(comparer(j, j + 1, param) > 0)
					swap(j, j + 1);
		}
	} else {
		for(int i = i2; i > i1; i--) {
			for(int j = i1; j < i; j++)
				if(comparer(j, j + 1, param) < 0)
					swap(j, j + 1);
		}
	}
}

void table::sort(int column, bool ascending) {
	table_sort_param e;
	e.element = this;
	e.column = column;
}

static void table_sort_column() {
	if(hot.param == current_sort_column)
		current_order = !current_order;
	else {
		current_sort_column = hot.param;
		current_order = true;
	}
	current_element->sort(hot.param, current_order);
}

void table::clickcolumn(int column) const {
	current_element = const_cast<table*>(this);
	draw::execute(table_sort_column, column);
}

int	column::get(const void* object) const {
	if(getnum)
		return getnum(object);
	return value.get(value.ptr((void*)object));
}

const char* column::get(const void* object, char* result, const char* result_end) const {
	if(getstr)
		return getstr(object, result, result_end);
	return value.gets(value.ptr((void*)object));
}

const char* column::gete(const void* object, char* result, const char* result_end) const {
	if(getstr)
		return getstr(object, result, result_end);
	auto v = value.get(value.ptr((void*)object));
	return "";
}

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
		if(!columns[column].is(ColumnVisible)
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

static void proc_mouseselect() {
	auto p = (table*)draw::hot.param;
	auto i = p->getvalid(p->current_hilite_column);
	p->select(p->current_hilite, i);
	if(p->columns[i].method->change_one_click)
		p->change(true);
}

void table::mouseselect(int id, bool pressed) {
	if(current_hilite == -1 || current_hilite_column == -1)
		return;
	if(pressed)
		draw::execute(proc_mouseselect, (int)this);
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
	draw::setclip({rc.x1, rc.y1, rc.x2, rc.y2 + 1});
	color active = colors::button.mix(colors::edit, 128);
	color a1 = active.lighten();
	color a2 = active.darken();
	rect r1;
	r1.x1 = rch.x1 - origin_width;
	r1.x2 = r1.x1;
	r1.y1 = rch.y1;
	r1.y2 = rch.y2;
	for(unsigned i = 0; i < columns.count; i++) {
		if(!columns[i].is(ColumnVisible))
			continue;
		if(dragactive(&columns[i])) {
		}
		r1.x2 = r1.x2 + columns[i].width;
		if(columns[i].size == SizeInner)
			continue;
		auto a = area(r1);
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
				dragbegin(&columns[i]);
				dragmouse.x = hot.mouse.x - r1.x1;
			}
		}
		r1.x1 = r1.x2;
	}
	return height;
}

int	table::gettotal(int column) const {
	auto& c = columns[column];
	auto type = c.total;
	if(!type)
		return 0;
	auto m = getmaximum();
	auto result = 0;
	switch(type) {
	case TotalMaximum:
		for(auto i = 0; i < m; i++) {
			auto v = c.get(get(i));
			if(v > result)
				result = v;
		}
		break;
	case TotalMinimum:
		result = 0x7FFFFFFF;
		for(auto i = 0; i < m; i++) {
			auto v = c.get(get(i));
			if(v < result)
				result = v;
		}
		break;
	default:
		for(auto i = 0; i < m; i++)
			result += c.get(get(i));
		if(type == TotalAverage)
			result = result / m;
		break;
	}
	return result;
}

void table::rowtotal(const rect& rc) const {
	const int header_padding = 4;
	char temp[260]; auto height = getrowheight();
	rect rch = {rc.x1, rc.y1, rc.x2, rc.y1 + height};
	color b1 = colors::button.lighten();
	color b2 = colors::button.darken();
	gradv(rch, b1, b2);
	rectb(rch, colors::border);
	draw::state push;
	draw::setclip({rc.x1, rc.y1, rc.x2, rc.y2 + 1});
	color active = colors::button.mix(colors::edit, 128);
	color a1 = active.lighten();
	color a2 = active.darken();
	rect r1;
	r1.x1 = rch.x1 - origin_width;
	r1.x2 = r1.x1;
	r1.y1 = rch.y1;
	r1.y2 = rch.y2;
	for(unsigned i = 0; i < columns.count; i++) {
		if(!columns[i].is(ColumnVisible))
			continue;
		r1.x2 = r1.x2 + columns[i].width;
		if(columns[i].size == SizeInner)
			continue;
		line(r1.x2, r1.y1, r1.x2, r1.y2, colors::border);
		auto result = gettotal(i);
		temp[0] = 0;
		if(result) {
			zprint(temp, "%1i", result);
			auto r2 = r1; r2.offset(4);
			draw::text(r2, temp, AlignRight | TextSingleLine);
		}
		r1.x1 = r1.x2;
	}
}

void table::row(const rect& rc, int index) {
	area(rc);
	if(select_mode == SelectRow)
		rowhilite(rc, index);
	auto current_column = getcolumn();
	auto x1 = rc.x1;
	auto need_tree = true;
	auto level_ident = 0;
	for(unsigned i = 0; i < columns.count; i++) {
		auto pc = columns.data + i;
		if(!pc->is(ColumnVisible))
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
		(this->*pc->method->render)(rt, index, i);
		x1 += rt.width() + 8;
	}
}

void table::ensurevisible() {
	list::ensurevisible();
	correction_width();
	auto rc = getrect(current, current_column);
	auto x1 = rc.x1 - (view_rect.x1 - origin_width);
	auto x2 = rc.x2 - (view_rect.x1 - origin_width);
	if(rc.x2 > view_rect.x2)
		origin_width = origin_width + rc.width();
	if(rc.x1 < view_rect.x1)
		origin_width = x1;
}

rect table::getrect(int row, int column) const {
	rect rs;
	rs.x1 = view_rect.x1 - origin_width;
	rs.x2 = rs.x1;
	rs.y1 = view_rect.y1 - origin + lines_per_page * row;
	rs.y2 = rs.y1 + getrowheight();
	if(column >= columns.getcount())
		column = columns.getcount() - 1;
	for(auto i = 0; i <= column; i++) {
		if(!columns[i].is(ColumnVisible))
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
		if(!columns[i].is(ColumnVisible))
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
	if(show_totals) {
		list::view({rc.x1, rc.y1, rc.x2, rc.y2 - getrowheight()});
		rowtotal({rc.x1, rc.y2 - getrowheight(), rc.x2, rc.y2});
	} else
		list::view(rc);
}

bool table::keyinput(unsigned id) {
	switch(id) {
	case KeyLeft:
		current_column = getvalid(current_column - 1, -1);
		ensurevisible();
		break;
	case KeyRight:
		current_column = getvalid(current_column + 1);
		ensurevisible();
		break;
	case KeyEnter:
		change(true);
		break;
	default: return list::keyinput(id);
	}
	return true;
}

column& table::addcol(const char* name, const char* type, const anyreq& value) {
	const visual* pf = 0;
	auto p = columns.add();
	memset(p, 0, sizeof(column));
	for(auto pp = getvisuals(); pp && *pp; pp++) {
		pf = (*pp)->find(type);
		if(pf)
			break;
	}
	if(pf) {
		p->method = pf;
		p->flags.add(ColumnVisible);
	} else
		p->method = visuals;
	p->title = szdup(name);
	p->value = value;
	p->size = p->method->size;
	p->width = p->method->default_width;
	p->total = p->method->total;
	p->align = p->method->align;
	if(!p->width)
		p->width = 100;
	current_column = getvalid(current_column, 1);
	return *p;
}

bool table::changefield(const rect& rc, unsigned flags, char* result, const char* result_maximum) {
	auto push_focus = getfocus();
	textedit te(result, result_maximum - result - 1, true);
	te.setfocus(true);
	te.show_border = false;
	te.post_escape = false;
	te.align = flags;
	te.rctext.x2++;
	auto r = te.editing({current_rect.x1, current_rect.y1, current_rect.x2, current_rect.y2});
	draw::setfocus(push_focus, true);
	return r;
}

void table::changenumber(const rect& rc, int line, int column) {
	char temp[32];
	zprint(temp, "%1i", columns[column].get(get(line)));
	if(changefield(rc, columns[column].align, temp, zendof(temp))) {
		auto& v = columns[current_column].value;
		v.set(v.ptr(get(line)), sz2num(temp));
	}
}

void table::changetext(const rect& rc, int line, int column) {
	char temp[8192];
	auto value = columns[column].value.gets(get(line));
	zcpy(temp, value, sizeof(temp) - 1);
	if(changefield(rc, columns[column].align, temp, zendof(temp))) {
		auto& v = columns[column].value;
		if(v.size == sizeof(const char*)) {
			if(temp[0])
				v.set(v.ptr(get(line)), (int)szdup(temp));
			else
				v.set(v.ptr(get(line)), 0);
		}
	}
}

void table::changecheck(const rect& rc, int line, int column) {
	auto r = columns[current_column].value;
	auto p = get(line);
	auto v = columns[column].get(p);
	auto b = 1 << r.bit;
	if((v & b) != 0)
		r.set(r.ptr(p), v & (~b));
	else
		r.set(r.ptr(p), v | b);
}

bool table::change(bool run) {
	if(read_only)
		return false;
	if(!columns)
		return false;
	if(current >= getmaximum())
		return false;
	if(columns[current_column].is(ColumnReadOnly))
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

void table::cell(const rect& rc, int line, int column, const char* ps) {
	if(!ps)
		return;
	auto align = columns[column].align;
	cellhilite(rc, line, column, ps, align);
	bool clipped = false;
	textc(rc.x1, rc.y1, rc.width(), ps, -1, align, &clipped);
	if(clipped) {
		if(areb(rc))
			tooltips(rc.x1, rc.y1, 200, ps);
	}
}

void table::cellhilite(const rect& rc, int line, int column, const char* text, image_flag_s aling) const {
	if(line == current && column == current_column) {
		rect rch = {rc.x1 - 4, rc.y1 - 4, rc.x2 + 3, rc.y2 + 3};
		switch(select_mode) {
		case SelectCell:
			hilight(rch);
			break;
		case SelectText:
			if(!text)
				break;
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

void table::cellenum(const rect& rc, int line, int column) {
	char temp[260];
	auto ps = columns[column].gete(get(line), temp, temp + sizeof(temp) / sizeof(temp[0]) - 1);
	cell(rc, line, column, ps);
}

void table::celltext(const rect& rc, int line, int column) {
	char temp[260];
	auto ps = columns[column].get(get(line), temp, temp + sizeof(temp) / sizeof(temp[0]) - 1);
	cell(rc, line, column, ps);
}

void table::cellimage(const rect& rc, int line, int column) {
	auto v = columns[column].get(get(line));
	auto s = gettreeimages();
	if(s)
		image(rc.x1 + rc.width() / 2, rc.y1 + rc.height() / 2, s, v, 0);
}

void table::cellrownumber(const rect& rc, int line, int column) {
	char temp[32]; zprint(temp, "%1i", line + 1);
	cell(rc, line, column, temp);
}

void table::cellnumber(const rect& rc, int line, int column) {
	char temp[32];
	zprint(temp, "%1i", columns[column].get(get(line)));
	cell(rc, line, column, temp);
}

void table::cellpercent(const rect& rc, int line, int column) {
	char temp[32];
	zprint(temp, "%1i%%", columns[column].get(get(line)));
	cell(rc, line, column, temp);
}

void table::celldate(const rect& rc, int line, int column) {
	datetime d(columns[column].get(get(line)));
	if(!d)
		return;
	char temp[260];
	zprint(temp, "%1.2i.%2.2i.%3.2i",
		d.day(), d.month(), d.year());
	cell(rc, line, column, temp);
}

void table::celldatetime(const rect& rc, int line, int column) {
	datetime d(columns[column].get(get(line)));
	if(!d)
		return;
	char temp[260];
	zprint(temp, "%1.2i.%2.2i.%3.2i %4.2i:%5.2i",
		d.day(), d.month(), d.year(), d.hour(), d.minute());
	cell(rc, line, column, temp);
}

void table::cellbox(const rect& rc, int line, int column) {
	unsigned flags = 0;
	auto v = columns[column].get(get(line));
	auto b = 1<<columns[column].value.bit;
	if(v&b)
		flags |= Checked;
	cellhilite(rc, line, column, 0, AlignCenter);
	clipart(rc.x1 + 2, rc.y1 + imax((rc.height() - 14) / 2, 0), 0, flags, ":check");
}

const visual** table::getvisuals() const {
	static const visual* elements[] = {visuals, 0};
	return elements;
}

const visual table::visuals[] = {{"number", "Числовое поле", AlignRight, 8, 80, SizeResized, TotalSummarize, &table::cellnumber, &table::changenumber, table::comparenum},
{"rownumber", "Номер рядка", AlignCenter, 8, 40, SizeResized, NoTotal, &table::cellrownumber},
{"checkbox", "Пометка", AlignCenter, 28, 28, SizeFixed, NoTotal, &table::cellbox, &table::changecheck, 0, true},
{"date", "Дата", AlignLeft, 8, 10 * 10 + 4, SizeResized, NoTotal, &table::celldate, 0, table::comparenum},
{"datetime", "Дата и время", AlignLeft, 8, 10 * 15 + 4, SizeResized, NoTotal, &table::celldatetime, 0, table::comparenum},
{"text", "Текстовое поле", AlignLeft, 8, 200, SizeResized, NoTotal, &table::celltext, &table::changetext, table::comparestr},
{"enum", "Перечисление", AlignLeft, 8, 200, SizeResized, NoTotal, &table::cellenum},
{"percent", "Процент", AlignRight, 40, 60, SizeResized, NoTotal, &table::cellpercent, &table::changenumber, table::comparenum},
{"image", "Изображение", AlignCenter, 20, 20, SizeInner, NoTotal, &table::cellimage, 0, table::comparenum},
{}};