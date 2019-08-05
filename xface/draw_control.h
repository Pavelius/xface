#include "anyval.h"
#include "bsreq.h"
#include "draw.h"
#include "markup.h"
#include "pointl.h"

#pragma once

namespace clipboard {
void					copy(const void* string, int lenght);
char*					paste();
}
namespace metrics {
namespace show {
extern bool				left, right, top, bottom;
extern bool				padding, statusbar;
}
}
namespace draw {
enum show_s : unsigned char {
	NoView, ViewIcon, ViewText, ViewIconAndText
};
enum column_size_s : unsigned char {
	SizeDefault,
	SizeResized, SizeFixed, SizeInner, SizeAuto,
};
enum select_mode_s : unsigned char {
	SelectCell, SelectText, SelectRow,
};
enum dock_s : unsigned char {
	DockLeft, DockLeftBottom,
	DockRight, DockRightBottom,
	DockBottom, DockWorkspace,
};
struct cmd {
	callback				proc;
	int						param;
	anyval					value;
	static cmd				ctx;
	constexpr cmd() : value(), proc(0), param(0) {}
	constexpr cmd(callback proc) : value(proc, 0), proc(proc), param(0) {}
	constexpr cmd(callback proc, int param) : value(proc, 0), proc(proc), param(param) {}
	constexpr cmd(callback proc, int param, void* data, unsigned size) : value(data, size), proc(proc), param(param) {}
	constexpr cmd(callback proc, int param, const anyval& value) : value(value), proc(proc), param(param) {}
	constexpr cmd(markup::commandproc proc, void* source) : value(proc, 0), proc(calling), param((int)source) {}
	static void				add();
	static void				assign();
	static void				calling();
	static void				invert();
	bool					ischecked() const { return ((value.get()&param)!=0) ? true : false; }
	void					execute() const { ctx = *this; draw::execute(proc, param); }
	int						getid() const { return (int)value.data; }
};
namespace controls {
struct control {
	typedef bool			(control::*callback)(bool run);
	struct command {
		struct builder {
			builder() = default;
			virtual ~builder() {}
			virtual void	add(const control::command& cmd) = 0;
			virtual void	addseparator() = 0;
			virtual const command* finish() const { return 0; }
			void			render(const control::command* commands);
			virtual void	start() const {}
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
	struct plugin {
		const char*			id;
		dock_s				dock;
		plugin*				next;
		static plugin*		first;
		plugin(const char* id, dock_s dock);
		virtual void		after_initialize() {}
		virtual void		before_render() {}
		static const plugin* find(const char* id);
		virtual control&	getcontrol() = 0;
	};
	bool					show_border;
	bool					show_background;
	bool					show_toolbar;
	static const sprite*	standart_toolbar;
	static const sprite*	standart_tree;
	constexpr control() : show_border(true), show_background(true), show_toolbar(true) {}
	virtual ~control() {}
	command::builder*		createmenu();
	void					execute(callback proc, int param = 0) const;
	const command*			getcommand(const char* id) const { return getcommands()->find(id); }
	virtual const command*	getcommands() const { return 0; }
	virtual const sprite*	getimages() const { return standart_toolbar; }
	virtual const char*		getlabel(char* result, const char* result_maximum) const { return 0; }
	virtual const sprite*	gettreeimages() const { return standart_tree; }
	virtual void			icon(int x, int y, bool disabled, const command& cmd) const;
	virtual bool			isdisabled() const { return false; }
	virtual bool			isfocusable() const { return true; }
	virtual bool			isfocused() const;
	bool					ishilited() const;
	virtual bool			keyinput(unsigned id); // Default behaivor call shortcut function
	virtual void			mouseinput(unsigned id, point mouse); // Default behaivor set focus
	virtual void			mousewheel(unsigned id, point mouse, int value) {}
	virtual void			redraw() {}
	virtual void			setfocus(bool instant);
	int						toolbar(int x, int y, int width) const;
	virtual void			view(const rect& rc);
};
// Analog of listbox element
struct list : control {
	int						origin, current, current_hilite, origin_width;
	int						lines_per_page, pixels_per_line, pixels_per_width;
	bool					show_grid_lines, show_selection, show_header;
	bool					hilite_odd_lines;
	rect					current_rect, view_rect;
	constexpr list() : origin(0), current(0), current_hilite(-1), origin_width(0),
		lines_per_page(0), pixels_per_line(0), pixels_per_width(0),
		show_grid_lines(false), show_selection(true), show_header(true), hilite_odd_lines(true),
		current_rect(), view_rect() {
	}
	void					correction();
	void					correction_width();
	virtual void			ensurevisible(); // Ånsure that current selected item was visible on screen if current 'count' is count of items per line
	int						find(int line, int column, const char* name, int lenght = -1) const;
	virtual int				getcolumn() const { return 0; } // Get current column
	virtual int				getident() const { return 6 * 2 + 4; } // get row ident in levels
	virtual int				getlevel(int index) const { return 0; } // get row ident in levels
	inline int				getline() const { return current; } // get current line
	int						getlinesperpage(int height) const { return height / pixels_per_line; }
	virtual const char*		getname(char* result, const char* result_max, int line, int column) const { return 0; }
	virtual int				getmaximum() const { return 0; }
	virtual int				getmaximumwidth() const { return 0; }
	static int				getrowheight(); // Get default row height for any List Control
	virtual int				getwidth(int column) const { return 0; } // Get column width
	void					hilight(const rect& rc, const rect* prc = 0) const;
	virtual bool			isgroup(int index) const { return false; }
	bool					isopen(int index) const;
	bool					keyinput(unsigned id) override;
	void					mousehiliting(const rect& rc, point mouse);
	virtual void			mouseinput(unsigned id, point position) override;
	virtual void			mouseselect(int id, bool pressed);
	virtual void			mousewheel(unsigned id, point position, int step) override;
	void					redraw() override;
	virtual void			row(const rect& rc, int index);
	virtual int				rowheader(const rect& rc) const { return 0; }
	virtual void			rowhilite(const rect& rc, int index) const;
	virtual void			select(int index, int column);
	void					treemark(rect rc, int index, int level) const;
	virtual bool			treemarking(bool run) { return true; }
	void					view(const rect& rc) override;
};
struct visual;
struct column {
	const visual*			method;
	const char*				id;
	const char*				title;
	int						width;
	const char*				tips;
	column_size_s			size;
	unsigned				flags;
	bool operator==(const char* value) const { return value && strcmp(id, value) == 0; }
	explicit operator bool() const { return method != 0; }
	bool					isvisible() const { return true; }
	bool					isreadonly() const { return (flags & Disabled) != 0; }
	column*					setreadonly() { if(!this) return 0; flags |= Disabled; return this; }
};
struct table : list {
	arem<column>			columns;
	int						current_column, current_hilite_column, current_column_maximum, maximum_width;
	bool					no_change_count;
	bool					no_change_order;
	bool					read_only;
	select_mode_s			select_mode;
	bool					show_totals;
	constexpr table() : current_column(0), current_column_maximum(0), current_hilite_column(-1), maximum_width(0),
		show_totals(false), no_change_order(false), no_change_count(false), read_only(false),
		select_mode(SelectCell) {
	}
	virtual column*			addcol(const char* id, const char* name, const char* type, column_size_s size = SizeDefault, int width = 0);
	void					cell(const rect& rc, int line, int column) const;
	void					cellbox(const rect& rc, int line, int column);
	void					cellimage(const rect& rc, int line, int column);
	void					cellhilite(const rect& rc, int line, int columen, const char* text, image_flag_s aling) const;
	void					cellnumber(const rect& rc, int line, int column);
	void					cellpercent(const rect& rc, int line, int column);
	void					celltext(const rect& rc, int line, int column);
	bool					change(bool run);
	virtual bool			changing(int line, int column, const char* name) { return false; }
	void					changecheck(const rect& rc, int line, int column);
	bool					changefield(const rect& rc, unsigned flags, char* result, const char* result_maximum);
	void					changenumber(const rect& rc, int line, int column);
	void					changetext(const rect& rc, int line, int column);
	virtual void			clickcolumn(int column) const {}
	virtual void			ensurevisible() override;
	virtual int				getcolumn() const override { return current_column; }
	virtual aref<column>	getcolumns() const { return aref<column>(); }
	virtual const char*		getheader(char* result, const char* result_maximum, int column) const { return columns[column].title; }
	virtual int				getnumber(int line, int column) const { return 0; }
	virtual int				getmaximumwidth() const { return maximum_width; }
	rect					getrect(int row, int column) const;
	virtual int				gettotal(int column) const { return 0; }
	virtual const char*		gettotal(char* result, const char* result_maximum, int column) const { return 0; }
	int						getvalid(int column, int direction = 1) const;
	virtual const visual*	getvisuals() const;
	bool					keyinput(unsigned id) override;
	void					mouseselect(int id, bool pressed) override;
	virtual void			row(const rect& rc, int index) override; // Draw single row - part of list
	virtual int				rowheader(const rect& rc) const override; // Draw header row
	virtual void			rowtotal(const rect& rc) const; // Draw header row
	void					select(int index, int column = 0) override;
	void					view(const rect& rc) override;
private:
	void					update_columns(const rect& rc);
};
struct visual {
	typedef void			(table::*proc_render)(const rect& rc, int line, int column);
	const char*				id;
	const char*				name;
	int						minimal_width, default_width;
	column_size_s			size;
	union {
		proc_render			render;
		const visual*		child;
	};
	proc_render				change;
	visual() = default;
	template<typename T, typename U> visual(const char* id, const char* name, int mw, int dw, column_size_s sz,
		void (T::*pr)(const rect& rc, int line, int column),
		void (U::*pc)(const rect& rc, int line, int column)) : id(id), name(name),
		render((proc_render)pr), change((proc_render)pc),
		size(sz), minimal_width(mw), default_width(dw) {
	}
	template<typename T> visual(const char* id, const char* name, int mw, int dw, column_size_s sz,
		void (T::*pr)(const rect& rc, int line, int column)) : id(id), name(name),
		render((proc_render)pr), change((proc_render)0),
		size(sz), minimal_width(mw), default_width(dw) {
	}
	constexpr visual(const visual* vs) : id("*"), name(""), change(0), minimal_width(0),
		default_width(0), child(vs), size(SizeDefault) {
	}
	explicit operator bool() const { return render != 0; }
	const visual*			find(const char* id) const;
};
// Abstract scrollable element. Scroll apear automatically if needed.
struct scrollable : control {
	pointl					origin;
	pointl					maximum;
	point					wheels;
	scrollable();
	rect					centerview(const rect& rc);
	virtual void			invalidate();
	virtual void			redraw(rect rc) {}
	void					view(const rect& rc) override;
};
struct textedit : scrollable {
	rect					rctext, rcclient;
	list*					records;
	unsigned				align;
	bool					readonly;
	bool					update_records;
	bool					show_records;
	bool					post_escape;
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
	void					setcount(int value) {}
	void					setrecordlist(const char* string);
	void					updaterecords(bool setfilter);
private:
	char*					string;
	unsigned				maxlenght;
	int						cashed_width;
	int						cashed_string;
	int						cashed_origin;
	int						p1, p2;
};
}
typedef void procchooselist(adat<void*, 64>& result, const bsreq** name_requisit, void* source);
void						application(bool allow_multiply_windows);
void						application(const char* name, bool allow_multiply_windows);
inline void					application() { application(true); }
void						application_initialize();
int							button(int x, int y, int width, unsigned flags, const cmd& ev, const char* label, const char* tips = 0, int key = 0);
int							checkbox(int x, int y, int width, unsigned flags, const cmd& ev, const char* label, const char* tips = 0);
int							checkbox(int x, int y, int width, bool& value, const char* label, const char* tips);
void						combobox(const rect& rc, const bsval& cmd);
int							combobox(int x, int y, int width, const char* header_label, const bsval& cmd, int header_width, const char* tips, procchooselist choose = 0);
void						dockbar(rect& rc);
bool						dropdown(const rect& rc, controls::control& e);
void						field(const rect& rco, unsigned flags, const anyval& ev, int digits, bstype_s type, callback choose_proc);
int							field(int x, int y, int width, const char* label, color& value, int header_width, const char* tips = 0);
int							field(int x, int y, int width, const char* label, const char*& sev, int header_width, callback choose_proc = 0);
int							field(int x, int y, int width, const char* label, const anyval& ev, int header_width, int digits);
int							field(int x, int y, int width, const markup* elements, const bsval& source, int title_width = 80);
int							radio(int x, int y, int width, unsigned flags, const cmd& cmd, const char* label, const char* tips = 0);
void						setposition(int& x, int& y, int& width, int padding = -1);
void						titletext(int& x, int y, int& width, unsigned flags, const char* label, int title, const char* separator = 0);
}
template<> struct bsmeta<draw::controls::control::plugin> {
	typedef draw::controls::control::plugin	data_type;
	static const bsreq		meta[];
};