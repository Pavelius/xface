#include "collection.h"
#include "draw_control.h"

#pragma once

namespace draw {
namespace controls {
struct grid : table, array {
	const bsreq*			type;
	constexpr grid(const bsreq* type, unsigned size) : type(type), array(size) {}
	constexpr grid(const bsreq* type, void* data, unsigned size, unsigned count_maximum) : type(type), array(data, size, count_maximum) {}
	constexpr grid(const bsreq* type, void* data, unsigned size, unsigned count_maximum, unsigned& count) : type(type),  array(data, size, count_maximum, count) {}
	constexpr grid(bsdata& manager) : type(manager.fields), array(manager.data, manager.size, manager.count_maximum, manager.count) {}
	template<typename T, unsigned N> constexpr grid(const bsreq* type, adat<T, N>& e) : grid(type, e.data, sizeof(T), N, e.count) {}
	template<typename T, unsigned N> constexpr grid(const bsreq* type, T(&e)[N]) : grid(type, e.data, sizeof(T), N) {}
	bool					add(bool run);
	virtual void			adding(void* value); // Default proc set all bits to zero
	bool					addcopy(bool run);
	bool					change(bool run);
	bool					changing(int line, int column, const char* name);
	void					clickcolumn(int column) const override;
	virtual void*			get(int index) const { return array::get(index); }
	const control::command* getcommands() const override;
	virtual const char*		getname(char* result, const char* result_max, int line, int column) const override;
	virtual int				getnumber(int line, int column) const override;
	virtual int				getmaximum() const override { return array::getcount(); }
	virtual bsval			getvalue(int row, int column) const;
	bool					keyinput(unsigned id) override;
	bool					movedown(bool run);
	bool					moveup(bool run);
	bool					remove(bool run);
	bool					setting(bool run);
	void					sort(int column, bool ascending);
	bool					sortas(bool run);
	bool					sortds(bool run);
};
struct gridref : grid {
	constexpr gridref(const bsreq* type, unsigned size=sizeof(void*)) : grid(type, size) {}
	void					add(void* object);
	void*					get(int index) const override;
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