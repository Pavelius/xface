#include "crt.h"
#include "screenshoot.h"
#include "stringcreator.h"
#include "draw_grid.h"

using namespace draw;
using namespace draw::controls;

static grid*	current_element;
static int		current_sort_column;
static int		current_order;

static int compare_column(const void* v1, const void* v2) {
	char t1[260], t2[260]; t1[0] = 0; t2[0] = 0;
	auto i1 = current_element->indexof(v1);
	auto i2 = current_element->indexof(v2);
	auto n1 = current_element->getname(t1, zendof(t1), i1, current_sort_column);
	auto n2 = current_element->getname(t2, zendof(t2), i2, current_sort_column);
	return strcmp(n1, n2)*current_order;
}

static void table_sort_column() {
	if(current_order == 1)
		current_order = -1;
	else
		current_order = 1;
	qsort(current_element->get(0), current_element->getcount(), current_element->getsize(), compare_column);
}

void grid::clickcolumn(int column) const {
	current_element = const_cast<grid*>(this);
	current_sort_column = column;
	draw::execute(table_sort_column);
}

bsval grid::getvalue(int row, int column) const {
	return {type->find(columns[column].id), get(row)};
}

int grid::getnumber(int line, int column) const {
	auto bv = getvalue(line, column);
	if(bv.type && bv.type->type == number_type)
		return bv.get();
	return 0;
}

const char* grid::getname(char* result, const char* result_max, int line, int column) const {
	auto bv = getvalue(line, column);
	if(!bv.type)
		return "";
	if(bv.type->type == number_type) {
		stringcreator sc;
		sc.prints(result, result_max, "%1i", getnumber(line, column));
		return result;
	} else if(bv.type->type == text_type) {
		auto p = (const char*)bv.get();
		if(p)
			return p;
		return "";
	} else
		return bv.getname();
}

bool grid::changing(int line, int column, const char* name) {
	auto bv = getvalue(line, column);
	if(!bv.type)
		return false;
	if(bv.type->type == number_type)
		bv.set(sz2num(name));
	else if(bv.type->type == text_type)
		bv.set((int)szdup(name));
	else
		return false;
	return true;
}

bool grid::add(bool run) {
	if(read_only)
		return false;
	if(getcount() >= (unsigned)getmaximum())
		return false;
	if(run) {
		select(indexof(array::add()));
		change(run);
	}
	return true;
}

bool grid::addcopy(bool run) {
	if(!add(false))
		return false;
	if(run) {
		auto c = array::get(current);
		auto p = array::add();
		if(p)
			memcpy(p, c, getsize());
		select(indexof(p));
		change(run);
	}
	return true;
}

static bool change_simple(const rect& rc, const bsval& bv, const char* tips) {
	draw::screenshoot screen;
	auto push_focus = getfocus();
	while(ismodal()) {
		screen.restore();
		draw::combobox(rc.x1, rc.y1, rc.width(), Focused, bv, 0, tips, 0);
		domodal();
		switch(hot.key) {
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
		case MouseLeft + Ctrl:
		case MouseLeft + Shift:
		case MouseLeftDBL:
		case MouseLeftDBL + Ctrl:
		case MouseLeftDBL + Shift:
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

bool grid::change(bool run) {
	if(read_only)
		return false;
	switch(columns[current_column].getcontol()) {
	case Field:
		if(run) {
			if(!current_rect)
				break;
			auto bv = getvalue(current, current_column);
			if(bv.type->type->issimple()) {
				auto push_focus = getfocus();
				char temp[8192]; auto pn = getname(temp, zendof(temp), current, current_column);
				if(pn != temp)
					zcpy(temp, pn, sizeof(temp) - 1);
				textedit te(temp, sizeof(temp) - 1, true);
				setfocus((int)&te, true);
				te.show_border = false;
				auto result = te.editing({current_rect.x1, current_rect.y1, current_rect.x2 + 1, current_rect.y2 + 1});
				setfocus(push_focus, true);
				if(result)
					changing(current, current_column, temp);
			} else {
				change_simple({current_rect.x1 - 4, current_rect.y1 - 4, current_rect.x2 + 4,
					current_rect.y2 + 1}, getvalue(current, current_column), 0);
			}
		}
		break;
	case Check:
		if(run) {
			if(getnumber(current, current_column))
				changing(current, current_column, "1");
			else
				changing(current, current_column, "0");
		}
		break;
	default:
		break;
	}
	return true;
}

bool grid::setting(bool run) {
	return true;
}

void grid::keyenter(int id) {
	change(true);
}

void grid::mouseleftdbl(point position) {
	change(true);
}

bool grid::moveup(bool run) {
	if(no_change_order)
		return false;
	if(current <= 0)
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

const control::command* grid::getcommands() const {
	static command add_elements[] = {{"add", "Добавить", 0, 0, &grid::add},
	{"addcopy", "Скопировать", 9, 0, &grid::addcopy},
	{"change", "Изменить", 10, F2, &grid::change},
	{}};
	static command move_elements[] = {{"moveup", "Переместить вверх", 21, 0, &grid::moveup},
	{"movedown", "Переместить вверх", 22, 0, &grid::movedown},
	{"sortas", "Сортировать по возрастанию", 11, 0, &grid::add},
	{"sortas", "Сортировать по убыванию", 12, 0, &grid::add},
	{}};
	static command elements[] = {{add_elements},
	{move_elements},
	{"setting", "Настройки", 16, 0, &grid::setting},
	{}};
	return elements;
}