#include "crt.h"
#include "stringcreator.h"
#include "draw_grid.h"

using namespace draw;
using namespace draw::controls;

static grid*	current_element;
static int		current_column;
static int		current_order;

static int compare_column(const void* v1, const void* v2) {
	char t1[260], t2[260]; t1[0] = 0; t2[0] = 0;
	auto i1 = current_element->indexof(v1);
	auto i2 = current_element->indexof(v2);
	auto n1 = current_element->getname(t1, zendof(t1), i1, current_column);
	auto n2 = current_element->getname(t2, zendof(t2), i2, current_column);
	return strcmp(n1, n2)*current_order;
}

static void table_sort_column() {
	if(current_order == 1)
		current_order = -1;
	else
		current_order = 1;
	qsort(current_element->get(0), current_element->getcount(), current_element->getsize(),
		compare_column);
}

void grid::clickcolumn(int column) const {
	current_element = const_cast<grid*>(this);
	current_column = column;
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
		auto number = getnumber(line, column);
		stringcreator sc;
		sc.prints(result, result_max, "%1i", number);
		return result;
	} else if(bv.type->type == text_type) {
		auto p = (const char*)bv.get();
		if(p)
			return p;
		return "";
	} else
		return bv.getname();
}