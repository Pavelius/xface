#include "bsreq.h"
#include "draw_control.h"

#pragma once

namespace draw {
namespace controls {
struct properties : control {
	struct translate {
		const char*		id;
		const char*		value;
		static int		compare(const void* v1, const void* v2);
	};
	int					title, spacing;
	void*				object;
	const bsreq*		type;
	arem<void*>			opened;
	aref<translate>		dictionary;
	properties() : title(80), spacing(0) { show_background = false; }
	void				addwidth(int& x, int& width, int v) const { x += v; width -= v; }
	void				clear() { object = 0; }
	void				close(void* object);
	bool				cmdopen(bool run);
	int					element(int x, int y, int width, void* object, const bsreq* type);
	void				focusfirst();
	const char*			getlabel(char* result, const char* result_maximum) const override { return "Свойства"; }
	virtual const char*	gettitle(char* result, const char* result_maximum, const void* object, const bsreq* type) const;
	int					group(int x, int y, int width, const char* label, void* object, const bsreq* type);
	bool				isfocusable() const override { return false; }
	bool				isopen(const void* object) const { return opened.is((void*)object); }
	virtual bool		isvisible(const void* object, const bsreq* type) const { return true; }
	void				open(void* object);
	void				set(void* object, const bsreq* type) { this->object = object; this->type = type; }
	void				treemark(int x, int y, int width, void* object, bool isopen) const;
	void				toggle(bool run);
	int					vertical(int x, int y, int width, void* object, const bsreq* type);
	void				view(const rect& rc) override;
};
}
}