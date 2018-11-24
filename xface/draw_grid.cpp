#include "crt.h"
#include "datetime.h"
#include "screenshoot.h"
#include "stringcreator.h"
#include "draw_grid.h"

using namespace draw;
using namespace draw::controls;

static int		current_sort_column;
static bool		current_order;
static grid*	current_element;

struct grid_sort_param {
	grid*		element;
	int			column;
};

static int compare_column_ascending(const void* v1, const void* v2, void* p) {
	auto pr = (grid_sort_param*)p;
	char t1[260], t2[260]; t1[0] = 0; t2[0] = 0;
	auto i1 = pr->element->indexof(v1);
	auto i2 = pr->element->indexof(v2);
	auto n1 = pr->element->getname(t1, zendof(t1), i1, pr->column);
	auto n2 = pr->element->getname(t2, zendof(t2), i2, pr->column);
	return strcmp(n1, n2);
}

static int compare_column_descending(const void* v1, const void* v2, void* p) {
	auto pr = (grid_sort_param*)p;
	char t1[260], t2[260]; t1[0] = 0; t2[0] = 0;
	auto i1 = pr->element->indexof(v1);
	auto i2 = pr->element->indexof(v2);
	auto n1 = pr->element->getname(t1, zendof(t1), i1, pr->column);
	auto n2 = pr->element->getname(t2, zendof(t2), i2, pr->column);
	return strcmp(n2, n1);
}

void grid::sort(int column, bool ascending) {
	grid_sort_param param;
	param.element = this;
	param.column = column;
	array::sort(0, getcount() - 1, ascending ? compare_column_ascending : compare_column_descending, &param);
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

static bool change_simple(const rect& rc, const bsval& bv, const char* tips) {
	draw::screenshoot screen;
	auto push_focus = getfocus();
	while(ismodal()) {
		screen.restore();
		draw::combobox(rc.x1, rc.y1, rc.width(), Focused, bv, 0, tips, 0);
		domodal();
		switch(hot.key & CommandMask) {
		case KeyEscape:
		case InputUpdate:
			breakmodal(0);
			break;
		case KeyTab:
		case KeyTab | Shift:
			breakmodal(0);
			execute(hot);
			break;
		case MouseLeft:
		case MouseLeftDBL:
			if(hot.pressed) {
				if(!areb(rc)) {
					breakmodal(0);
					execute(hot);
				}
			}
			break;
		}
	}
	setfocus(push_focus, true);
	return getresult() != 0;
}

void grid::clickcolumn(int column) const {
	current_element = const_cast<grid*>(this);
	draw::execute(table_sort_column, column);
}

bsval grid::getvalue(int row, int column) const {
	return bsval(get(row), type).get(columns[column].id);
}

int grid::getnumber(int line, int column) const {
	auto bv = getvalue(line, column);
	if(bv && bv.type->type == number_type)
		return bv.get();
	return 0;
}

const char* grid::getname(char* result, const char* result_max, int line, int column) const {
	auto bv = getvalue(line, column);
	if(!bv)
		return "";
	if(bv.type->type == number_type) {
		stringcreator sc;
		auto value = getnumber(line, column);
		sc.print(result, result_max, "%1i", value);
		return result;
	} else if(bv.type->type == text_type) {
		auto p = (const char*)bv.get();
		if(p)
			return p;
		return "";
	} else
		return bv.dereference().getname();
}

void grid::changeref(const rect& rc, int line, int column) {
	change_simple({rc.x1 - 4, rc.y1 - 4, rc.x2 + 4, rc.y2 + 3}, getvalue(line, column), 0);
}

bool grid::changing(int line, int column, const char* name) {
	auto bv = getvalue(line, column);
	if(!bv)
		return false;
	if(bv.type->type == number_type)
		bv.set(sz2num(name));
	else if(bv.type->type == text_type)
		bv.set((int)szdup(name));
	else
		return false;
	return true;
}

void grid::adding(void* new_row) {
	memset(new_row, 0, getsize());
}

bool grid::add(bool run) {
	if(read_only)
		return false;
	if(no_change_count)
		return false;
	if(getcount() >= (unsigned)getmaxcount())
		return false;
	if(run) {
		auto p = array::add();
		adding(p);
		select(indexof(p));
		redraw();
		change(run);
	}
	return true;
}

bool grid::addcopy(bool run) {
	if(!add(false))
		return false;
	if(!getcount())
		return false;
	if(run) {
		auto c = array::get(current);
		auto p = array::add();
		if(p)
			memcpy(p, c, getsize());
		select(indexof(p));
		redraw();
		change(true);
	}
	return true;
}

bool grid::setting(bool run) {
	return true;
}

bool grid::keyinput(unsigned id) {
	switch(id) {
	case KeyEnter: change(true); break;
	default: return table::keyinput(id);
	}
	return true;
}

bool grid::moveup(bool run) {
	if(no_change_order)
		return false;
	if(current <= 0)
		return false;
	if(getcount() == 1)
		return false;
	if(run) {
		swap(current - 1, current);
		select(current - 1, getcolumn());
	}
	return true;
}

bool grid::movedown(bool run) {
	if(no_change_order)
		return false;
	if(current >= int(getcount() - 1))
		return false;
	if(run) {
		swap(current + 1, current);
		select(current + 1, getcolumn());
	}
	return true;
}

bool grid::remove(bool run) {
	if(read_only)
		return false;
	if(no_change_count)
		return false;
	if(!getcount())
		return false;
	if(run)
		array::remove(current, 1);
	return true;
}

bool grid::sortas(bool run) {
	if(no_change_order)
		return false;
	if(getmaximum() <= 1)
		return false;
	if(run)
		sort(current_column, true);
	return true;
}

bool grid::sortds(bool run) {
	if(no_change_order)
		return false;
	if(getmaximum() <= 1)
		return false;
	if(run)
		sort(current_column, false);
	return true;
}

void grid::celldate(const rect& rc, int line, int column) {
	char temp[260]; temp[0] = 0;
	datetime d(getnumber(line, column));
	if(d) {
		szprint(temp, zendof(temp), "%1.2i.%2.2i.%3.2i",
			d.day(), d.month(), d.year());
		cellhilite(rc, line, column, temp, AlignLeft);
		draw::text(rc, temp, AlignLeft);
	}
}

void grid::celldatetime(const rect& rc, int line, int column) {
	char temp[260]; temp[0] = 0;
	datetime d(getnumber(line, column));
	if(d) {
		szprint(temp, zendof(temp), "%1.2i.%2.2i.%3.2i %4.2i:%5.2i",
			d.day(), d.month(), d.year(), d.hour(), d.minute());
		cellhilite(rc, line, column, temp, AlignLeft);
		draw::text(rc, temp, AlignLeft);
	}
}

const visual* grid::getvisuals() const {
	static visual elements[] = {{table::getvisuals()},
	{"date", "Дата", 8, textw("0") * 10 + 4, SizeResized, &grid::celldate},
	{"datetime", "Дата и время", 8, textw("0") * 15 + 4, SizeResized, &grid::celldatetime},
	{"ref", "Ссылка", 8, 200, SizeResized, &table::celltext, &grid::changeref},
	{}};
	return elements;
}

const control::command* grid::getcommands() const {
	static command add_elements[] = {{"add", "Добавить", 0, 0, &grid::add},
	{"addcopy", "Скопировать", 9, 0, &grid::addcopy},
	{"change", "Изменить", 10, F2, &grid::change},
	{"remove", "Удалить", 19, KeyDelete, &grid::remove},
	{}};
	static command move_elements[] = {{"moveup", "Переместить вверх", 21, 0, &grid::moveup},
	{"movedown", "Переместить вверх", 22, 0, &grid::movedown},
	{"sortas", "Сортировать по возрастанию", 11, 0, &grid::sortas},
	{"sortas", "Сортировать по убыванию", 12, 0, &grid::sortds},
	{}};
	static command elements[] = {{add_elements},
	{move_elements},
	{"setting", "Настройки", 16, 0, &grid::setting},
	{}};
	return elements;
}

void gridref::add(void* object) {
	array::add(&object);
}

void* gridref::get(int index) const {
	return *((void**)array::get(index));
}