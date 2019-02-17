#include "draw_control.h"

#pragma once

namespace draw {
namespace controls {
struct properties : control {
	int					title, spacing;
	bsval				value;
	properties() : title(80), spacing(0) { show_background = false; }
	void				clear() { value.data = 0; value.type = 0; }
	int					element(int x, int y, int width, const bsval& ev);
	const char*			getlabel(char* result, const char* result_maximum) const override { return "Свойства"; }
	virtual const char*	gettitle(char* result, const char* result_maximum, const bsval& ev) const { return ev.type->id; }
	bool				isfocusable() const override { return false; }
	virtual bool		isvisible(const bsval& v) const { return true; }
	void				set(const bsval& v) { value = v; }
	int					vertical(int x, int y, int width, const bsval& ev);
	void				view(const rect& rc) override;
};
}
}
