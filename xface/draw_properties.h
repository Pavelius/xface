#include "draw_control.h"

#pragma once

namespace draw {
namespace controls {
struct properties : control {
	int					page, page_maximum;
	void*				pages[32];
	int					title, spacing;
	void*				object;
	const markup*		type;
	properties() : title(80), spacing(0) { show_background = false; }
	void				addwidth(int& x, int& width, int v) const { x += v; width -= v; }
	void				clear() { object = 0; }
	int					element(int x, int y, int width, void* object, const markup* type);
	void				focusfirst();
	virtual fntext		getfntext(const void* object, const markup* type) const { return type->list.getname; }
	virtual fnallow		getfnallow(const void* object, const markup* type) const { return type->list.allow; }
	const char*			getlabel(stringbuilder& sb) const override { return "Свойства"; }
	virtual const char*	gettitle(stringbuilder& sb, const void* object, const markup* type) const;
	bool				isfocusable() const override { return false; }
	virtual bool		isvisible(const void* object, const markup* type) const { return true; }
	void				set(void* object, const markup* type) { this->object = object; this->type = type; }
	int					group(int x, int y, int width, void* object, const markup* type);
	void				view(const rect& rc) override;
};
}
}