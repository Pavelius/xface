#include "codemodel.h"
#include "draw_control.h"

#pragma once

namespace draw {
namespace controls {
struct codeview : control, codemodel {
	int						cash_origin;
	int						lines_per_page;
	int						p1, p2;
	point					pos1, pos2, size;
	point					origin, maximum;
	const char*				nextstep(const char* ps, int dir);
public:
	rect					rctext, rcclient;
	bool					readonly;
	static const sprite*	font;
	static point			fontsize;
	codeview();
	void					beforeredraw(const rect& rc);
	void					clear();
	bool					copy(bool run);
	void					correction();
	bool					cut(bool run);
	void					ensurevisible(int linenumber);
	int						getbegin() const;
	point					getbeginpos() const;
	command*				getcommands() const override;
	int						getcurrent() const { return p1; }
	point					getcurrentpos() const { return pos1; }
	int						getend() const;
	point					getendpos() const;
	int						getpixelperline() const { return fontsize.y; }
	int						hittest(rect rc, point pt, unsigned state) const;
	static void				instance();
	void					invalidate();
	bool					isselected() const { return p2 != -1 && p1 != -1; }
	bool					keyinput(unsigned id) override;
	void					left(bool shift, bool ctrl);
	int						linen(int index) const;
	void					mouseinput(unsigned id, point position);
	void					open(const char* url);
	bool					paste(bool run);
	void					paste(const char* input);
	void					redraw(const rect& rc);
	void					right(bool shift, bool ctrl);
	void					set(int index, bool shift);
	unsigned				select_all(bool run);
	void					setvalue(const char* id, int value) override;
	void					view(const rect& rc) override;
	void					updatestate(const point origin, int& origin_index) { getstate(p1, pos1, p2, pos2, size, origin, origin_index); }
};
}
}
