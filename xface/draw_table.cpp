#include "crt.h"
#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

static int		current_sort_column;
static bool		current_order;
static table*	current_element;

int table::comparest(int i1, int i2, int column) const {
	char t1[260], t2[260]; t1[0] = 0; t2[0] = 0;
	auto n1 = columns[column].get(get(i1), t1, t1 + sizeof(t1) - 1);
	auto n2 = columns[column].get(get(i2), t2, t2 + sizeof(t2) - 1);
	return strcmp(n1, n2);
}

int table::comparenm(int i1, int i2, int column) const {
	auto pc = &columns[column];
	return pc->get(get(i1)) - pc->get(get(i2));
}

int table::comparer(int i1, int i2, const sortparam* param, int count) const {
	for(auto i = 0; i < count; i++) {
		auto pc = columns[param[i].column].method->comparer;
		if(!pc)
			continue;
		auto r = (this->*pc)(i1, i2, param[i].column);
		if(r == 0)
			continue;
		return r * param[i].multiplier;
	}
	return 0;
}

void table::sort(int i1, int i2, sortparam* ps, int count) {
	if(i2 <= i1)
		return;
	for(int i = i2; i > i1; i--) {
		for(int j = i1; j < i; j++)
			if(comparer(j, j + 1, ps, count) > 0)
				swap(j, j + 1);
	}
}

void table::sort(int column, bool ascending) {
	sortparam e;
	e.column = column;
	e.multiplier = ascending ? 1 : -1;
	sort(0, getmaximum() - 1, &e, 1);
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
	if(!type)
		return 0;
	return type->get(type->ptr(object));
}

void column::set(const void* object, int v) {
	if(!type)
		return;
	type->set(type->ptr(object), v);
}

const char* column::get(const void* object, char* result, const char* result_end) const {
	if(getpresent)
		return getpresent(object, result, result_end);
	if(type)
		return type->gets(type->ptr(object));
	return result;
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

void* table::getcurrent() const {
	if(current >= getmaximum())
		return 0;
	return get(current);
}

static void proc_mouseselect() {
	auto p = (table*)draw::hot.param;
	auto i = p->getvalid(list::current_hilite_column);
	p->select(list::current_hilite_row, i);
	if(p->columns[i].method->change_one_click)
		p->change(true);
}

void table::mouseselect(int id, bool pressed) {
	if(!ishilited() || current_hilite_row == -1 || current_hilite_column == -1)
		return;
	if(pressed)
		draw::execute(proc_mouseselect, (int)this);
}

int table::rowheader(const rect& rc) const {
	static int base_width;
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
	r1.x2 = r1.x1; r1.x2++;
	r1.y1 = rch.y1;
	r1.y2 = rch.y2;
	for(unsigned i = 0; i < columns.count; i++) {
		if(!columns[i].is(ColumnVisible))
			continue;
		if(dragactive(&columns[i])) {
			hot.cursor = CursorLeftRight;
			auto new_width = base_width + (hot.mouse.x - dragmouse.x);
			auto pw = columns[i].method;
			if(new_width < pw->minimal_width)
				new_width = pw->minimal_width;
			columns.data[i].width = new_width;
		}
		r1.x2 = r1.x2 + columns[i].width;
		if(columns[i].size == SizeInner)
			continue;
		auto a = ishilite(r1);
		if(!no_change_order) {
			if(a) {
				if(hot.pressed)
					gradv({r1.x1 + 1, r1.y1 + 1, r1.x2, r1.y2}, a2, a1);
				else
					gradv({r1.x1 + 1, r1.y1 + 1, r1.x2, r1.y2}, a1, a2);
				if(hot.key == MouseLeft && !hot.pressed)
					clickcolumn(i);
			}
		}
		// Нарисуем границу только когда она далеко от края
		// Чтобы она была не видна, если ширина элемента впритык к краю.
		line(r1.x2, r1.y1, r1.x2, r1.y2, colors::border);
		temp[0] = 0;
		auto p = getheader(temp, temp + sizeof(temp) / sizeof(temp[0]) - 1, i);
		if(p)
			textc(r1.x1 + metrics::edit.x1, r1.y1 + metrics::edit.y1, r1.width() + metrics::edit.width(), p);
		a = ishilite({r1.x2 + metrics::edit.x2 - 1, r1.y1, r1.x2 + metrics::edit.x2 + 1, r1.y2});
		if(a) {
			hot.cursor = CursorLeftRight;
			if(hot.pressed && hot.key == MouseLeft) {
				dragbegin(&columns[i]);
				base_width = columns[i].width;
				dragmouse.x = hot.mouse.x;
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
	char temp[260]; auto height = getrowheight();
	rect rch = {rc.x1, rc.y1, rc.x2, rc.y1 + height};
	color b1 = colors::button.lighten();
	color b2 = colors::button.darken();
	gradv(rch, b1, b2);
	rectb(rch, colors::border);
	draw::state push;
	draw::setclip({rc.x1, rc.y1, rc.x2, rc.y2 + 1});
	rect r1;
	r1.x1 = rch.x1 - origin_width;
	r1.x2 = r1.x1; r1.x2++;
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
			auto r2 = r1 + metrics::edit;
			draw::text(r2, temp, AlignRight | TextSingleLine);
		}
		r1.x1 = r1.x2;
	}
}

void table::row(const rect& rc, int index) {
	ishilite(rc);
	if(select_mode == SelectRow)
		rowhilite(rc, index);
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
		rect rt = {x1 + metrics::edit.x1,
			rc.y1 + metrics::edit.y1,
			x1 + pc->width + metrics::edit.x2,
			rc.y2 + metrics::edit.y2};
		if(level_ident) {
			if(columns[i].size != SizeInner && columns[i].size != SizeFixed) {
				auto mx = rt.width();
				if(mx > level_ident)
					mx = level_ident;
				rt.x2 -= mx;
				level_ident -= mx;
			}
		}
		if(show_grid_lines && columns[i].size != SizeInner) {
			auto x = x1 + pc->width;
			draw::line(x, rt.y1 - metrics::edit.y1, x, rt.y2 - metrics::edit.y2, colors::border);
		}
		ishilite(rt);
		(this->*pc->method->render)(rt, index, i);
		x1 += pc->width;
	}
}

void table::ensurevisible() {
	list::ensurevisible();
	correction_width();
	auto rc = getrect(current, current_column);
	auto x1 = rc.x1 - (view_rect.x1 - origin_width);
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

static const char* get_visual_default(const bsreq* type) {
	if(!type)
		return 0;
	if(type->type == bsmeta<int>::meta)
		return "number";
	else if(type->type == bsmeta<const char*>::meta)
		return "text";
	else if(type->type == bsmeta<datetime>::meta)
		return "datetime";
	return "enum";
}

column& table::addcol(const bsreq* metadata, const char* id, const char* name, const char* visual_id) {
	const visual* pf = 0;
	auto p = columns.add();
	memset(p, 0, sizeof(column));
	p->type = metadata->find(id);
	if(!visual_id)
		visual_id = get_visual_default(p->type);
	if(visual_id) {
		for(auto pp = getvisuals(); pp && *pp; pp++) {
			pf = (*pp)->find(visual_id);
			if(pf)
				break;
		}
	}
	if(pf) {
		p->method = pf;
		p->flags.add(ColumnVisible);
	} else
		p->method = visuals;
	p->title = szdup(name);
	p->size = p->method->size;
	p->width = p->method->default_width;
	if(p->width < 0)
		p->width = -p->width * draw::textw('0') + 4;
	p->total = p->method->total;
	p->align = p->method->align;
	if(p->type)
		p->source = p->type->source;
	if(!p->width)
		p->width = 100;
	current_column = getvalid(current_column, 1);
	return *p;
}

column& table::addstdimage() {
	return addcol(0, "image", 0, "standart_image");
}

bool table::changefield(const rect& rc, unsigned flags, char* result, const char* result_maximum) {
	pushfocus pf;
	textedit te(result, result_maximum - result - 1, true);
	te.setfocus(true);
	te.show_border = false;
	te.post_escape = false;
	te.align = flags;
	return te.editing(current_rect);
}

void table::changenumber(const rect& rc, int line, int column) {
	char temp[32];
	zprint(temp, "%1i", columns[column].get(get(line)));
	if(changefield(rc, columns[column].align, temp, zendof(temp)))
		columns[column].set(get(line), sz2num(temp));
}

void table::changetext(const rect& rc, int line, int column) {
	char temp[8192];
	auto value = (const char*)columns[column].get(get(line));
	if(!value)
		temp[0] = 0;
	else
		zcpy(temp, value, sizeof(temp) - 1);
	if(changefield(rc, columns[column].align, temp, zendof(temp)))
		columns[column].set(get(line), (int)szdup(temp));
}

const char* table::getenumid(const void* object, char* result, const char* result_maximum) {
	if(!object)
		return "Нет";
	return ((enumi*)object)->id;
}

const char* table::getenumname(const void* object, char* result, const char* result_maximum) {
	if(!object)
		return "Пусто";
	return ((enumi*)object)->name;
}

void table::changeref(const rect& rc, int line, int column) {
	if(!columns[column].source)
		return;
	auto p = get(line);
	const anyval av((char*)p + columns[column].type->offset, columns[column].type->size, 0);
	fieldm(rc, av, *columns[column].source, getenumname, true, 0, 0);
}

void table::changecheck(const rect& rc, int line, int column) {
	auto p = get(line);
	auto v = columns[column].get(p);
	auto b = 1 << columns[column].param;
	if((v & b) != 0)
		columns[column].set(p, v & (~b));
	else
		columns[column].set(p, v | b);
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
		if(ishilite(rc))
			tooltips(rc.x1, rc.y1, 200, ps);
	}
}

void table::cellhilite(const rect& rc, int line, int column, const char* text, image_flag_s aling) const {
	if(line == current && column == current_column) {
		rect rch = {rc.x1 - metrics::edit.x1, rc.y1 - metrics::edit.y1, rc.x2 - metrics::edit.x2, rc.y2 - metrics::edit.y2 - 1};
		switch(select_mode) {
		case SelectCell:
			hilight({rch.x1 + 1, rch.y1, rch.x2, rch.y2}, &rch);
			break;
		case SelectText:
			if(!text)
				break;
			switch(aling) {
			case AlignRight:
			case AlignRightCenter:
			case AlignRightBottom:
				hilight({rc.x2 - metrics::edit.x1 - draw::textw(text) + 1, rc.y1 - metrics::edit.y1,
					rc.x2 - metrics::edit.x2 + 1, rc.y2 - metrics::edit.y2 + 1}, &rch);
				break;
			default:
				hilight({rc.x1 - metrics::edit.x1, rc.y1 - metrics::edit.y1,
					rc.x1 + draw::textw(text) - metrics::edit.x2 + 1,
					rc.y2 - metrics::edit.y2 - 1}, &rch);
				break;
			}
			break;
		default:
			break;
		}
	}
}

void table::celltext(const rect& rc, int line, int column) {
	char temp[260];
	auto ps = columns[column].get(get(line), temp, temp + sizeof(temp) / sizeof(temp[0]) - 1);
	cell(rc, line, column, ps);
}

void table::cellimagest(const rect& rc, int line, int column) {
	auto s = gettreeimages();
	if(!s)
		return;
	auto v = getimage(line);
	if(v == -1)
		return;
	image(rc.x1 + rc.width() / 2, rc.y1 + rc.height() / 2, s, v, 0);
}

void table::cellimage(const rect& rc, int line, int column) {
	auto s = gettreeimages();
	if(!s)
		return;
	auto v = columns[column].get(get(line));
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
	auto b = 1 << columns[column].param;
	if(v&b)
		flags |= Checked;
	cellhilite(rc, line, column, 0, AlignCenter);
	clipart(rc.x1 + 2, rc.y1 + imax((rc.height() - 14) / 2, 0), 0, flags, ":check");
}

const visual** table::getvisuals() const {
	static const visual* elements[] = {visuals, 0};
	return elements;
}

bool table::sortas(bool run) {
	if(no_change_order)
		return false;
	if(getmaximum() <= 1)
		return false;
	if(run)
		sort(current_column, true);
	return true;
}

bool table::sortds(bool run) {
	if(no_change_order)
		return false;
	if(getmaximum() <= 1)
		return false;
	if(run)
		sort(current_column, false);
	return true;
}

bool table::moveup(bool run) {
	if(no_change_order)
		return false;
	if(current <= 0)
		return false;
	if(getmaximum() == 1)
		return false;
	if(run) {
		swap(current - 1, current);
		select(current - 1, getcolumn());
	}
	return true;
}

bool table::movedown(bool run) {
	if(no_change_order)
		return false;
	if(current >= int(getmaximum() - 1))
		return false;
	if(run) {
		swap(current + 1, current);
		select(current + 1, getcolumn());
	}
	return true;
}

bool table::addrow(bool run) {
	if(no_change_count)
		return false;
	if(run) {
		auto p = addrow();
		if(p) {
			select(current + 1, getcolumn());
			change(true);
		}
	}
	return true;
}

bool table::removerow(bool run) {
	if(read_only)
		return false;
	if(no_change_count)
		return false;
	if(!getmaximum())
		return false;
	if(run)
		remove(current);
	return true;
}

bool table::setting(bool run) {
	return true;
}

control::command table::commands_add[] = {{"add", "Добавить", 0, &table::addrow, 9},
{"change", "Изменить", 0, &table::change, 10, F2},
{"remove", "Удалить", 0, &table::removerow, 19, KeyDelete},
{}};
control::command table::commands_move[] = {{"moveup", "Переместить вверх", 0, &table::moveup, 21},
{"movedown", "Переместить вниз", 0, &table::movedown, 22},
{"sortas", "Сортировать по возрастанию", 0, &table::sortas, 11},
{"sortds", "Сортировать по убыванию", 0, &table::sortds, 12},
{}};
control::command table::commands[] = {{"*", "", commands_add, &table::isaddable},
{"*", "", commands_move, &table::ismoveable},
{"setting", "Настройки", 0, &table::setting, 16, 0},
{}};
const visual table::visuals[] = {{"number", "Числовое поле", AlignRight, 8, 80, SizeResized, TotalSummarize, &table::cellnumber, &table::changenumber, &table::comparenm},
{"rownumber", "Номер рядка", AlignCenter, 8, 40, SizeResized, NoTotal, &table::cellrownumber},
{"checkbox", "Пометка", AlignCenter, 28, 28, SizeFixed, NoTotal, &table::cellbox, &table::changecheck, 0, true},
{"date", "Дата", AlignLeft, 8, -12, SizeResized, NoTotal, &table::celldate, 0, &table::comparenm},
{"datetime", "Дата и время", AlignLeft, 8, -16, SizeResized, NoTotal, &table::celldatetime, 0, &table::comparenm},
{"text", "Текстовое поле", AlignLeft, 8, 200, SizeResized, NoTotal, &table::celltext, &table::changetext, &table::comparest},
{"enum", "Перечисление", AlignLeft, 8, 200, SizeResized, NoTotal, &table::celltext, &table::changeref},
{"percent", "Процент", AlignRight, 40, 60, SizeResized, TotalAverage, &table::cellpercent, &table::changenumber, &table::comparenm},
{"image", "Изображение", AlignCenter, 20, 20, SizeInner, NoTotal, &table::cellimage, 0, &table::comparenm},
{"standart_image", "Стандартное изображение", AlignCenter, 20, 20, SizeInner, NoTotal, &table::cellimagest, 0, 0},
{}};