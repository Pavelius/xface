#include "codemodel.h"
#include "draw_control.h"

#pragma once

namespace draw {
namespace controls {
struct codeview : public scrollable, arem<char> {
	int						cashed_width;
	int						cashed_string;
	int						cashed_origin;
public:
	rect					rctext, rcclient;
	bool					readonly;
	//
	virtual void			cashing(rect rc);
	bool					copy(bool run);
	void					correct();
	bool					cut(bool run);
	bool					editing(rect rc);
	void					ensurevisible(int linenumber);
	command*				getcommands() const override;
	point					getpos(rect rc, int index, unsigned state) const;
	int						getrecordsheight() const;
	int						hittest(rect rc, point pt, unsigned state) const;
	void					invalidate() override;
	bool					keyinput(unsigned id) override;
	int						lineb(int index) const;
	int						linee(int index) const;
	int						linen(int index) const;
	void					left(bool shift, bool ctrl);
	void					mouseinput(unsigned id, point position);
	bool					paste(bool run);
	void					redraw(const rect& rc) override;
	void					right(bool shift, bool ctrl);
	void					select(int index, bool shift);
	unsigned				select_all(bool run);
	void					setrecordlist(const char* string);
	void					updaterecords(bool setfilter);
};
}
}
