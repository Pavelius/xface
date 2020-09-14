#include "codemodel.h"
#include "draw_control.h"

#pragma once

namespace draw {
namespace controls {
struct codeview : public scrollable, codemodel {
	int						cash_columns;
	void					textout(int x, int y, int index);
public:
	rect					rctext, rcclient;
	bool					readonly;
	static const sprite*	font;
	static point			fontsize;
	codeview();
	void					beforeredraw() override;
	void					changing() override { invalidate(); }
	bool					copy(bool run);
	bool					cut(bool run);
	void					ensurevisible(int linenumber);
	command*				getcommands() const override;
	int						hittest(rect rc, point pt, unsigned state) const;
	static void				instance();
	void					invalidate() override;
	bool					keyinput(unsigned id) override;
	int						linen(int index) const;
	void					mouseinput(unsigned id, point position);
	void					open(const char* url);
	bool					paste(bool run);
	void					redraw(const rect& rc) override;
	unsigned				select_all(bool run);
	void					setvalue(const char* id, int value) override;
};
}
}
