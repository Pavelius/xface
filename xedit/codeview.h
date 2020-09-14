#include "codemodel.h"
#include "draw_control.h"

#pragma once

namespace draw {
namespace controls {
struct codeview : public scrollable, codemodel {
	int						cash_columns;
	int						lines_per_page;
	int						p1, p2;
	point					pos1, pos2, size;
	void					textout(int x, int y, int index);
public:
	rect					rctext, rcclient;
	bool					readonly;
	static const sprite*	font;
	static point			fontsize;
	codeview();
	void					beforeredraw(const rect& rc) override;
	void					clear();
	bool					copy(bool run);
	void					correction();
	bool					cut(bool run);
	void					ensurevisible(int linenumber);
	int						getbegin() const;
	point					getbeginpos() const;
	command*				getcommands() const override;
	int						getcurrent() const { return p1; }
	int						getend() const;
	point					getendpos() const;
	int						getindex(int origin, int column, int row) const;
	point					getpos(int index) const;
	int						hittest(rect rc, point pt, unsigned state) const;
	static void				instance();
	void					invalidate() override;
	bool					isselected() const { return p2 != -1 && p1 != -1; }
	bool					keyinput(unsigned id) override;
	void					left(bool shift, bool ctrl);
	int						linen(int index) const;
	void					mouseinput(unsigned id, point position);
	void					open(const char* url);
	bool					paste(bool run);
	void					paste(const char* input);
	void					redraw(const rect& rc) override;
	void					right(bool shift, bool ctrl);
	void					set(int index, bool shift);
	unsigned				select_all(bool run);
	void					setvalue(const char* id, int value) override;
	void					updatestate() { getstate(p1, pos1, p2, pos2, size); }
};
}
}
