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
	constexpr grid(bsdata& manager) : type(manager.meta), array(manager.data, manager.size, manager.maximum, manager.count) {}
	template<typename T, unsigned N> constexpr grid(const bsreq* type, adat<T, N>& e) : grid(type, e.data, sizeof(T), N, e.count) {}
	template<typename T, unsigned N> constexpr grid(const bsreq* type, T(&e)[N]) : grid(type, e.data, sizeof(T), N) {}
	bool					add(bool run);
	bool					addcopy(bool run);
	virtual void			adding(void* value); // Default proc set all bits to zero
	void					celldate(const rect& rc, int line, int column);
	void					celldatetime(const rect& rc, int line, int column);
	void					changeref(const rect& rc, int line, int column);
	bool					changing(int line, int column, const char* name) override;
	void					clickcolumn(int column) const override;
	virtual void*			get(int index) const { return array::get(index); }
	const control::command* getcommands() const override;
	virtual const char*		getname(char* result, const char* result_max, int line, int column) const override;
	virtual int				getnumber(int line, int column) const override;
	virtual int				getmaximum() const override { return array::getcount(); }
	bsval					getvalue(int row, int column) const;
	const visual*			getvisuals() const override { return standart_visuals; }
	visual**				getvisualsparent() const override { return standart_visualsparent; }
	bool					keyinput(unsigned id) override;
	bool					movedown(bool run);
	bool					moveup(bool run);
	bool					remove(bool run);
	bool					setting(bool run);
	void					sort(int column, bool ascending);
	bool					sortas(bool run);
	bool					sortds(bool run);
	static visual			standart_visuals[];
	static visual*			standart_visualsparent[];
};
struct gridref : grid {
	constexpr gridref(const bsreq* type, unsigned size=sizeof(void*)) : grid(type, size) {}
	void					add(const void* object);
	void*					get(int index) const override;
	void*					getcurrent() const { return getcount() ? get(current) : 0; }
};
struct tree : grid {
	struct element {
		unsigned char		level;
		unsigned char		flags;
		unsigned char		type;
		unsigned char		image;
		void*				object;
	};
	struct builder {
		int					index;
		unsigned char		level;
		tree*				pc;
		constexpr builder(tree* pc, int index, unsigned char level) : pc(pc), index(index), level(level) {}
		void*				add(void* object, unsigned char image, unsigned char type, bool group);
	};
	bool					sort_rows_by_name;
	constexpr tree(const bsreq* type, unsigned size = sizeof(element)) : grid(type, size), sort_rows_by_name(false) {}
	void					collapse(int index);
	void					expand(int index, int level);
	virtual void			expanding(builder& e) {}
	int						find(const void* value) const;
	void*					get(int index) const override;
	int						getblockcount(int index) const;
	const control::command* getcommands() const;
	int						getimage(int index) const;
	int						getlevel(int index) const override;
	int						getnext(int index, int increment = 1) const;
	int						getnumber(int line, int column) const override;
	int						getparent(int index) const;
	int						getroot(int index) const;
	int						gettreecolumn() const;
	int						gettype(int index) const;
	bool					isgroup(int index) const override;
	bool					keyinput(unsigned id) override;
	bool					moveup(bool run);
	bool					movedown(bool run);
	void					open(int max_level);
	bool					remove(bool run);
	void					shift(int i1, int i2);
	void					toggle(int index);
	bool					treemarking(bool run) override;
};
}
}