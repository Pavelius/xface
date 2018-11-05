#include "bsreq.h"
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
struct column {
	unsigned			flags;
	const char*			id;
	const char*			title;
	int					width;
	const char*			tips;
	bool operator==(const char* value) const { return value && strcmp(id, value) == 0; }
	explicit operator bool() const { return id != 0; }
	draw_event_s		getcontol() const { return (draw_event_s)(flags&ControlMask); }
	bool				isvisible() const { return true; }
};
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
	};
	bool				show_border;
	bool				show_background;
	static const sprite* standart_toolbar;
	constexpr control(): show_border(true), show_background(true) {}
	void				execute(callback proc) const;
	const command*		getcommand(const char* id) const;
	virtual const command* getcommands() const { return 0; }
	virtual const sprite* getimages() const { return standart_toolbar; }
	virtual void		icon(int x, int y, bool disabled, const command& cmd) const;
	virtual bool		isfocusable() const { return true; }
	bool				isfocused() const;
	bool				ishilited() const;
	virtual void		keybackspace(int id) {}
	virtual void		keydelete(int id) {}
	virtual void		keydown(int id) {}
	virtual void		keyend(int id) {}
	virtual void		keyenter(int id) {}
	virtual void		keyleft(int id) {}
	virtual void		keyhome(int id) {}
	virtual void		keypageup(int id) {}
	virtual void		keypagedown(int id) {}
	virtual void		keyright(int id) {}
	virtual void		keysymbol(int symbol) {}
	virtual void		keyup(int id) {}
	virtual void		mouseleft(point position); // Default behaivor set focus
	virtual void		mouseleftdbl(point position) {}
	virtual void		mousewheel(point position, int step) {}
	int					toolbar(int x, int y, int width) const;
	bool				translate(unsigned key);
	virtual void		view(rect rc);
};
// Analog of listbox element
struct list : control {
	int					origin, current, current_hilite;
	int					lines_per_page, pixels_per_line;
	bool				show_grid_lines, show_selection;
	bool				hilite_odd_lines;
	rect				current_rect;
	constexpr list() : origin(0), current(0), current_hilite(-1),
		lines_per_page(0), pixels_per_line(0), show_grid_lines(false), show_selection(true), hilite_odd_lines(true),
		current_rect() {}
	void				correction();
	void				ensurevisible(); // ensure that current selected item was visible on screen if current 'count' is count of items per line
	int					find(int line, int column, const char* name, int lenght = -1) const;
	virtual int			getcolumn() const { return 0; } // get current column
	inline int			getline() const { return current; } // get current line
	int					getlinesperpage(int height) const { return height / pixels_per_line; }
	virtual const char* getname(char* result, const char* result_max, int line, int column) const { return 0; }
	virtual int			getmaximum() const { return 0; }
	virtual int			getmaximumwidth() const { return 0; }
	virtual int			getoriginwidth() const { return 0; }
	static int			getrowheight(); // Get default row height for any List Control
	virtual int			getwidth(int column) const { return 0; } // get column width
	void				hilight(rect rc) const;
	void				keydown(int id) override;
	void				keyend(int id) override;
	void				keyenter(int id) override;
	void				keyhome(int id) override;
	void				keypageup(int id) override;
	void				keypagedown(int id) override;
	void				keysymbol(int symbol) override;
	void				keyup(int id) override;
	void				mousehiliting(const rect& rc, point mouse);
	void				mouseleftdbl(point position) override;
	void				mousewheel(point position, int step) override;
	virtual void		select(int index, int column);
	virtual void		mouseselect(int id, bool pressed);
	virtual void		row(rect rc, int index) const; // Draw single row - part of list
	virtual void		rowhilite(rect rc, int index) const;
	void				view(rect rc) override;
};
struct table : list {
	const column*		columns;
	int					origin_width, current_column, current_hilite_column, current_column_maximum, maximum_width;
	bool				no_change_count;
	bool				no_change_order;
	bool				read_only;
	bool				select_full_row;
	bool				show_totals;
	bool				show_header;
	constexpr table(const column* columns) : columns(columns), origin_width(0), current_column(0), current_column_maximum(0), current_hilite_column(-1), maximum_width(0),
		show_totals(false), show_header(true),
		no_change_order(false), no_change_count(false), read_only(false),
		select_full_row(false) {}
	virtual void		clickcolumn(int column) const {}
	virtual void		custom(char* buffer, const char* buffer_maximum, const rect& rc, int line, int column) const {}
	virtual int			getcolumn() const override { return current_column; }
	virtual const char*	getheader(char* result, const char* result_maximum, int column) const { return columns[column].title; }
	virtual int			getnumber(int line, int column) const { return 0; }
	virtual int			getmaximumwidth() const { return maximum_width; }
	virtual int			gettotal(int column) const { return 0; }
	virtual const char*	gettotal(char* result, const char* result_maximum, int column) const { return 0; }
	void				keyleft(int id) override;
	void				keyright(int id) override;
	void				mouseselect(int id, bool pressed) override;
	virtual void		row(rect rc, int index) const override; // Draw single row - part of list
	virtual int			rowheader(rect rc) const; // Draw header row
	void				select(int index, int column = 0) override;
	void				view(rect rc) override;
	void				viewtotal(rect rc) const;
};
// Abstract scrollable element. Scroll apear automatically if needed.
struct scrollable : control {
	pointl				origin;
	pointl				maximum;
	point				wheels;
	scrollable();
	rect				centerview(rect rc);
	virtual void		invalidate();
	virtual void		redraw(rect rc) {}
	void				view(rect rc) override;
};
struct textedit : scrollable {
	char*				string;
	unsigned			maxlenght;
	int					p1, p2;
	rect				rctext, rcclient;
	list*				records;
	unsigned			align;
	bool				readonly;
	bool				update_records;
	bool				show_records;
	//
	textedit(char* string, unsigned maximum_lenght, bool select_text);
	//
	void				clear();
	virtual void		cashing(rect rc);
	unsigned			copy(bool run);
	void				correct();
	bool				editing(rect rc);
	void				ensurevisible(int linenumber);
	int					getrecordsheight() const;
	int					hittest(rect rc, point pt, unsigned state) const;
	void				keysymbol(int symbol) override;
	void				invalidate() override;
	bool				isshowrecords() const;
	void				keybackspace(int id) override;
	void				keydelete(int id) override;
	void				keydown(int id) override;
	void				keyend(int id) override;
	void				keyhome(int id) override;
	void				keyleft(int id) override;
	void				keyright(int id) override;
	void				keyup(int id) override;
	int					lineb(int index) const;
	int					linee(int index) const;
	int					linen(int index) const;
	void				left(bool shift, bool ctrl);
	int					getbegin() const;
	int					getend() const;
	point				getpos(rect rc, int index, unsigned state) const;
	unsigned			paste(bool run);
	void				paste(const char* string);
	void				redraw(rect rc) override;
	void				right(bool shift, bool ctrl);
	void				select(int index, bool shift);
	unsigned			select_all(bool run);
	void				setrecordlist(const char* string);
	void				updaterecords(bool setfilter);
protected:
	int					cashed_width;
	int					cashed_string;
	int					cashed_origin;
};
}
int						button(int x, int y, int width, unsigned flags, const runable& cmd, const char* label, const char* tips = 0, int key = 0);
int						checkbox(int x, int y, int width, unsigned flags, const runable& cmd, const char* label, const char* tips = 0);
int						combobox(int x, int y, int width, unsigned flags, const bsval& cmd, const char* label, const char* tips, int header_width);
int						field(int x, int y, int width, unsigned flags, const cmdfd& cmd, const char* label, const char* tips, int header_width);
int						radio(int x, int y, int width, unsigned flags, const runable& cmd, const char* label, const char* tips = 0);
int						render(int x, int y, int width, const bsval& value, const widget* element);
void					setposition(int& x, int& y, int& width, int padding = -1);
void					titletext(int& x, int y, int& width, unsigned flags, const char* label, int title);
bool					dropdown(const rect& rc, controls::control& e);
}