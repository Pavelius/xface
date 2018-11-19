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
	bool					addcopy(bool run);
	virtual void			adding(void* value); // Default proc set all bits to zero
	void					changeref(const rect& rc, int line, int column);
	bool					changing(int line, int column, const char* name) override;
	void					clickcolumn(int column) const override;
	virtual void*			get(int index) const { return array::get(index); }
	const control::command* getcommands() const override;
	virtual const char*		getname(char* result, const char* result_max, int line, int column) const override;
	virtual int				getnumber(int line, int column) const override;
	virtual int				getmaximum() const override { return array::getcount(); }
	virtual bsval			getvalue(int row, int column) const;
	virtual const visual*	getvisuals() const override;
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
		unsigned char		flags;
		unsigned char		type;
		unsigned char		image;
		unsigned			param;
	};
	bool					sort_rows_by_name;
	constexpr tree(const bsreq* type, unsigned size = sizeof(element)) : grid(type, size), sort_rows_by_name(false) {}
	void					collapse(int index);
	virtual void			expand(int index, int level) {}
	int						findbyparam(int value) const;
	void*					get(int index) const override;
	int						getimage(int index) const;
	int						getlastchild(int index) const;
	int						getlevel(int index) const;
	int						getparam(int index) const;
	int						getparent(int index) const;
	int						getroot(int index) const;
	int						gettype(int index) const;
	//bool					haselement(int param) const;
	bool					isgroup(int index) const override;
	void					open(int max_level);
	void					toggle(int index);
};
}
}