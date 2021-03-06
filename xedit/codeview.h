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
	pointl					origin, maximum;
	rect					rctext;
	bool					readonly;
	static const sprite*	font;
	static point			fontsize;
	codeview();
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
	const char*				nextstep(const char* ps, int dir);
	void					open(const char* url);
	bool					paste(bool run);
	void					paste(const char* input);
	void					pastecr();
	void					pastetab();
	void					redraw(const rect& rc);
	void					right(bool shift, bool ctrl);
	void					set(int index, bool shift);
	unsigned				select_all(bool run);
	void					setvalue(const char* id, int value) override;
	bool					wordselect(bool run);
	void					view(const rect& rc) override;
};
}
}
