#include "draw_control.h"

#pragma once

namespace draw {
namespace controls {
struct properties : control {
	int					title, spacing;
	bsval				value;
	arem<void*>			opened;
	properties() : title(80), spacing(0) { show_background = false; }
	void				addwidth(int& x, int& width, int v) const { x += v; width -= v; }
	void				clear() { value.data = 0; value.type = 0; }
	void				close(void* object);
	int					element(int x, int y, int width, const bsval& ev);
	const char*			getlabel(char* result, const char* result_maximum) const override { return "Свойства"; }
	virtual const char*	gettitle(char* result, const char* result_maximum, const bsval& ev) const { return ev.type->id; }
	int					group(int x, int y, int width, int ident, const char* label, const bsval& ev) const;
	bool				isfocusable() const override { return false; }
	bool				isopen(const void* object) const { return opened.is((void*)object); }
	virtual bool		isvisible(const bsval& v) const { return true; }
	void				open(void* object);
	void				set(const bsval& v) { value = v; }
	void				treemark(int x, int y, int width, bool isopen) const;
	int					vertical(int x, int y, int width, const bsval& ev);
	void				view(const rect& rc) override;
};
}
}
