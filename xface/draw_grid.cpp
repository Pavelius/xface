#include "stringcreator.h"
#include "draw_grid.h"

using namespace draw;
using namespace draw::controls;

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
	} else {
		auto p = (void*)bv.get();
		if(p)
			return bsval(bv.type->type, p).getname();
		return "";
	}
}