#include "collection.h"
#include "draw_control.h"

#pragma once

namespace draw {
namespace controls {
struct grid : table, array {
	const bsreq*			type;
	bool					select_full_row;
	//
	grid(const column* columns, const bsreq* type, const array& source) : table(columns), type(type), array(source), can_grow(false) {}
	bool					add(bool run);
	bool					addcopy(bool run);
	bool					change(bool run);
	void					clickcolumn(int column) const override;
	const control::command* getcommands() const override;
	virtual const char*		getname(char* result, const char* result_max, int line, int column) const override;
	virtual int				getnumber(int line, int column) const override;
	virtual int				getmaximum() const override { return array::getcount(); }
	virtual bsval			getvalue(int row, int column) const;
	bool					setting(bool run);
private:
	bool					can_grow;
};
}
}
