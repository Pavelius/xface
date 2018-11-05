#include "collection.h"
#include "draw_control.h"

#pragma once

namespace draw {
namespace controls {
struct grid : table, array {
	const bsreq*			type;
	bool					read_only;
	grid(const column* columns, const bsreq* type, unsigned size) : table(columns), type(type), array(size), read_only(false) {}
	grid(const column* columns, const bsreq* type, void* data, unsigned size, unsigned count_maximum) : table(columns), type(type), read_only(false), array(data, size, count_maximum) {}
	grid(const column* columns, const bsreq* type, void* data, unsigned size, unsigned count_maximum, unsigned& count) : table(columns), type(type), read_only(false), array(data, size, count_maximum, count) {}
	template<typename T, unsigned N> grid(const column* columns, const bsreq* type, adat<T, N>& e) : grid(columns, type, e.data, sizeof(T), N, e.count) {}
	template<typename T, unsigned N> grid(const column* columns, const bsreq* type, T(&e)[N]) : grid(columns, type, e.data, sizeof(T), N) {}
	bool					add(bool run);
	bool					addcopy(bool run);
	bool					change(bool run);
	bool					changing(int line, int column, const char* name);
	void					clickcolumn(int column) const override;
	const control::command* getcommands() const override;
	virtual const char*		getname(char* result, const char* result_max, int line, int column) const override;
	virtual int				getnumber(int line, int column) const override;
	virtual int				getmaximum() const override { return array::getcount(); }
	virtual bsval			getvalue(int row, int column) const;
	virtual void*			getrow(int index) { return get(index); }
	void					keyenter(int id) override;
	bool					movedown(bool run);
	bool					moveup(bool run);
	void					mouseleftdbl(point position) override;
	bool					setting(bool run);
};
struct tree : grid {
	struct element {
		unsigned char		level;
		unsigned char		image;
		unsigned short		flags;
		void*				param;
	};
};
}
}
