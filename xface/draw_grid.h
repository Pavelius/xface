#include "collection.h"
#include "draw_control.h"

#pragma once

namespace draw {
namespace controls {
struct grid : table, avec {
	const bsreq*			type;
	//
	grid(const column* columns, const bsreq* type, const avec& source) : table(columns), type(type), avec(source) {}
	void					clickcolumn(int column) const override;
	bsval					getvalue(int row, int column) const;
	virtual const char*		getname(char* result, const char* result_max, int line, int column) const override;
	virtual int				getnumber(int line, int column) const override;
	virtual int				getmaximum() const override { return avec::getcount(); }
};
}
}
