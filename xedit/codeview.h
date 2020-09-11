#include "codemodel.h"
#include "draw_control.h"

#pragma once

namespace draw {
namespace controls {
struct codeview : public scrollable, codemodel {
	int						cashed_width;
	int						cashed_string;
	int						cashed_origin;
	void					textout(int x, int y, int index);
public:
	rect					rctext, rcclient;
	bool					readonly;
	static sprite*			font;
	//
	void					changing() override { invalidate(); }
	bool					copy(bool run);
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
	void					mouseinput(unsigned id, point position);
	bool					paste(bool run);
	void					redraw(const rect& rc) override;
	unsigned				select_all(bool run);
	void					setrecordlist(const char* string);
	void					setvalue(const char* id, int value) override;
	void					updaterecords(bool setfilter);
};
}
}
