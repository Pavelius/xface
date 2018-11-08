#include "bsdata.h"
#include "draw.h"
#include "pointl.h"

#pragma once

namespace clipboard {
void					copy(const void* string, int lenght);
char*					paste();
}
namespace draw {
struct runable {
	virtual int			getid() const = 0;
	virtual void		execute() const = 0;
};
struct cmd : runable {
	constexpr cmd(callback_proc id, int param = 0) : id(id), param(param) {}
	virtual void		execute() const override { draw::execute(id, param); }
	virtual int			getid() const override { return (int)id; }
private:
	callback_proc		id;
	int					param;
};
struct cmdfd : runable {
	virtual bool		choose(bool run) const { return false; }
	virtual bool		clear(bool run) const { return false; }
	virtual bool		dropdown(const rect& rc, bool run) const { return false; }
	virtual const char*	get(char* result, const char* result_maximum, bool force_result) const { result[0] = 0; return result; }
	virtual bool		increment(int step, bool run) const { return false; }
	virtual bool		open(bool run) const { return false; }
	virtual void		set(const rect& value) const {}
};
struct widget {
	unsigned			flags;
	const char*			id;
	const char*			label;
	int					value;
	int					width;
	int					height;
	int					title;
	const widget*		childs;
	const char*			tips;
	int					key;
	void				(*proc)();
	operator bool() const { return flags != 0; }
};
namespace controls {
struct control {
	enum show_s : unsigned char { NoView, ViewIcon, ViewText, ViewIconAndText };
	typedef bool			(control::*callback)(bool run);
	struct command {
		struct builder {
			virtual void	add(const control::command& cmd) = 0;
			virtual void	addseparator() = 0;
			void			render(const control::command* commands);
		private:
			void			render(const control::command* commands, bool& separator, int& count);
		};
		const char*			id;
		const char*			name;
		union {
			callback		proc;
			const command*	child;
		};
		unsigned			key;
		int					image;
		show_s				view;
		constexpr command() : id(0), name(0), proc(0), key(0), image(0), view(ViewIcon) {}
		constexpr command(const command* child) : id("*"), name(0), child(child), key(0), image(0), view(ViewIcon) {}
		template<class T> command(const char* id, const char* name, int image, unsigned key, bool (T::*proc)(bool run)) : id(id), name(name), proc((callback)proc), key(key), image(image), view(ViewIcon) {}
		explicit operator bool() const { return id != 0; }
		const command*		find(const char* id) const;
		const command*		find(unsigned key) const;
	};
	bool					show_border;
	bool					show_background;
	static const sprite*	standart_toolbar;
	constexpr control() : show_border(true), show_background(true) {}
	virtual ~control() {}
	void					execute(callback proc) const;
	const command*			getcommand(const char* id) const { return getcommands()->find(id); }
	virtual const command*	getcommands() const { return 0; }
	virtual const sprite*	getimages() const { return standart_toolbar; }
	virtual void			icon(int x, int y, bool disabled, const command& cmd) const;
	virtual bool			isfocusable() const { return true; }
	bool					isfocused() const;
	bool					ishilited() const;
	virtual bool			keyinput(unsigned id); // Default behaivor call shortcut function
	virtual void			mouseinput(unsigned id, point mouse); // Default behaivor set focus
	virtual void			mousewheel(unsigned id, point mouse, int value) {}
	int						toolbar(int x, int y, int width) const;
	virtual void			view(rect rc);
};
struct visual {
	typedef void			(control::*proc_render)(const rect& rc, int line, int column) const;
	typedef void			(control::*proc_change)(const rect& rc, int line, int column);
	const char*				id;
	const char*				name;
	int						minimal_width, default_width;
	union {
		proc_render			render;
		const visual*		child;
	};
	proc_change				change;
	visual() = default;
	template<typename T, typename U> visual(const char* id, const char* name, int mw, int dw,
		(T::*pr)(const rect& rc, int line, int column) const,
		(U::*pc)(const rect& rc, int line, int column)) : id(id), name(name),
		render((proc_render)pr), change((proc_change)pc), minimal_width(mw), default_width(dw) {}
	constexpr visual(const visual* vs) : id("*"), name(""), change(0), minimal_width(0), default_width(0), child(vs) {}
	explicit operator bool() const { return render != 0; }
	const visual*			find(const char* id) const;
};
struct column {
	const visual*			method;
	const char*				id;
	const char*				title;
	int						width;
	const char*				tips;
	bool operator==(const char* value) const { return value && strcmp(id, value) == 0; }
	explicit operator bool() const { return method != 0; }
	bool					isvisible() const { return true; }
};
// Analog of listbox element
struct list : control {
	int						origin, current, current_hilite, origin_width;
	int						lines_per_page, pixels_per_line;
	bool					show_grid_lines, show_selection;
	bool					hilite_odd_lines;
	rect					current_rect;
	constexpr list() : origin(0), current(0), current_hilite(-1), origin_width(0),
		lines_per_page(0), pixels_per_line(0), show_grid_lines(false), show_selection(true), hilite_odd_lines(true),
		current_rect() {}
	void					correction();
	void					ensurevisible(); // �nsure that current selected item was visible on screen if current 'count' is count of items per line
	int						find(int line, int column, const char* name, int lenght = -1) const;
	virtual int				getcolumn() const { return 0; } // Get current column
	inline int				getline() const { return current; } // get current line
	int						getlinesperpage(int height) const { return height / pixels_per_line; }
	virtual const char*		getname(char* result, const char* result_max, int line, int column) const { return 0; }
	virtual int				getmaximum() const { return 0; }
	virtual int				getmaximumwidth() const { return 0; }
	static int				getrowheight(); // Get default row height for any List Control
	virtual int				getwidth(int column) const { return 0; } // Get column width
	void					hilight(rect rc) const;
	bool					keyinput(unsigned id) override;
	void					mousehiliting(const rect& rc, point mouse);
	virtual void			select(int index, int column);
	virtual void			mouseinput(unsigned id, point position) override;
	virtual void			mouseselect(int id, bool pressed);
	virtual void			mousewheel(unsigned id, point position, int step) override;
	virtual void			row(rect rc, int index) const;
	virtual void			rowhilite(rect rc, int index) const;
	void					view(rect rc) override;
};
struct table : list {
	arem<column>			columns;
	int						current_column, current_hilite_column, current_column_maximum, maximum_width;
	bool					no_change_count;
	bool					no_change_order;
	bool					read_only;
	bool					select_full_row;
	bool					show_totals;
	bool					show_header;
	constexpr table() : current_column(0), current_column_maximum(0), current_hilite_column(-1), maximum_width(0),
		show_totals(false), show_header(true),
		no_change_order(false), no_change_count(false), read_only(false),
		select_full_row(false),
		view_rect() {}
	virtual column*			addcol(const char* id, const char* name, const char* type, int width = 0);
	bool					change(bool run);
	virtual bool			changing(int line, int column, const char* name) { return false; }
	void					changecheck(const rect& rc, int line, int column);
	bool					changefield(const rect& rc, unsigned flags, char* result, const char* result_maximum);
	void					changenumber(const rect& rc, int line, int column);
	void					changetext(const rect& rc, int line, int column);
	void					checkbox(const rect& rc, int line, int column) const;
	virtual void			clickcolumn(int column) const {}
	void					fieldnumber(const rect& rc, int line, int column) const;
	void					fieldpercent(const rect& rc, int line, int column) const;
	void					fieldtext(const rect& rc, int line, int column) const;
	virtual int				getcolumn() const override { return current_column; }
	virtual aref<column>	getcolumns() const { return aref<column>(); }
	virtual const char*		getheader(char* result, const char* result_maximum, int column) const { return columns[column].title; }
	virtual int				getnumber(int line, int column) const { return 0; }
	virtual int				getmaximumwidth() const { return maximum_width; }
	virtual int				gettotal(int column) const { return 0; }
	virtual const char*		gettotal(char* result, const char* result_maximum, int column) const { return 0; }
	virtual const visual*	getvisuals() const;
	bool					keyinput(unsigned id) override;
	void					mouseselect(int id, bool pressed) override;
	void					redraw();
	virtual void			row(rect rc, int index) const override; // Draw single row - part of list
	virtual int				rowheader(rect rc) const; // Draw header row
	void					select(int index, int column = 0) override;
	void					view(rect rc) override;
	void					viewtotal(rect rc) const;
private:
	rect					view_rect;
};
// Abstract scrollable element. Scroll apear automatically if needed.
struct scrollable : control {
	pointl					origin;
	pointl					maximum;
	point					wheels;
	scrollable();
	rect					centerview(rect rc);
	virtual void			invalidate();
	virtual void			redraw(rect rc) {}
	void					view(rect rc) override;
};
struct textedit : scrollable {
	char*					string;
	unsigned				maxlenght;
	int						p1, p2;
	rect					rctext, rcclient;
	list*					records;
	unsigned				align;
	bool					readonly;
	bool					update_records;
	bool					show_records;
	//
	textedit(char* string, unsigned maximum_lenght, bool select_text);
	//
	void					clear();
	virtual void			cashing(rect rc);
	unsigned				copy(bool run);
	void					correct();
	bool					editing(rect rc);
	void					ensurevisible(int linenumber);
	int						getrecordsheight() const;
	int						hittest(rect rc, point pt, unsigned state) const;
	void					invalidate() override;
	bool					isshowrecords() const;
	bool					keyinput(unsigned id) override;
	int						lineb(int index) const;
	int						linee(int index) const;
	int						linen(int index) const;
	void					left(bool shift, bool ctrl);
	int						getbegin() const;
	int						getend() const;
	point					getpos(rect rc, int index, unsigned state) const;
	unsigned				paste(bool run);
	void					paste(const char* string);
	void					redraw(rect rc) override;
	void					right(bool shift, bool ctrl);
	void					select(int index, bool shift);
	unsigned				select_all(bool run);
	void					setrecordlist(const char* string);
	void					updaterecords(bool setfilter);
protected:
	int						cashed_width;
	int						cashed_string;
	int						cashed_origin;
};
}
int							button(int x, int y, int width, unsigned flags, const runable& cmd, const char* label, const char* tips = 0, int key = 0);
int							checkbox(int x, int y, int width, unsigned flags, const runable& cmd, const char* label, const char* tips = 0);
int							combobox(int x, int y, int width, unsigned flags, const bsval& cmd, const char* label, const char* tips, int header_width);
int							field(int x, int y, int width, unsigned flags, const cmdfd& cmd, const char* label, const char* tips, int header_width);
int							radio(int x, int y, int width, unsigned flags, const runable& cmd, const char* label, const char* tips = 0);
int							render(int x, int y, int width, const bsval& value, const widget* element);
void						setposition(int& x, int& y, int& width, int padding = -1);
void						titletext(int& x, int y, int& width, unsigned flags, const char* label, int title);
bool						dropdown(const rect& rc, controls::control& e);
}