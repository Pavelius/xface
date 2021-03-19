#include "anyval.h"
#include "crt.h"
#include "datetime.h"
#include "draw.h"
#include "markup.h"
#include "pointl.h"

#pragma once

namespace clipboard {
void						copy(const void* string, int lenght);
char*						paste();
}
namespace metrics {
namespace show {
extern bool					left, right, top, bottom;
extern bool					padding, statusbar;
}
}
namespace draw {
enum show_s : unsigned char {
	ViewIcon, ViewText, ViewIconAndText
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
	DockBottom, DockWorkspace
};
enum column_s : unsigned char {
	ColumnVisible, ColumnReadOnly,
};
enum total_s : unsigned char {
	NoTotal,
	TotalSummarize, TotalMaximum, TotalMinimum, TotalAverage
};
struct docki {
	const char*				id;
	const char*				name;
};
class pushfocus {
	anyval					value;
public:
	pushfocus();
	~pushfocus();
};
namespace controls {
struct control {
	typedef bool			(control::*fncmd)(bool run);
	typedef bool			(control::*fnvisible)() const;
	struct proci {
		fncmd				cmd;
		fnevent				cmd_event;
		fnvisible			visible;
		constexpr proci() : cmd(0), cmd_event(0), visible(0) {}
		constexpr proci(fnevent v) : cmd(0), cmd_event(v), visible(0) {}
		template<class T> constexpr proci(bool (T::*v)() const) : cmd(0), cmd_event(0), visible((fnvisible)v) {}
		template<class T> constexpr proci(bool (T::*v)(bool run)) : cmd((fncmd)v), cmd_event(0), visible(0) {}
	};
	struct plugin {
		struct builder {
			virtual bool	canopen(const char* url) const;
			virtual control* create(const char* url) = 0;
			virtual void	destroy(control* v) {}
			virtual void	getextensions(stringbuilder& sb) const {}
			virtual bool	read(const char* url) { return false; }
		};
		const char*			id;
		dock_s				dock;
		bool				visible;
		plugin*				next;
		static plugin*		first;
		plugin(const char* id, dock_s dock);
		static const plugin* find(const char* id);
		virtual control*	getcontrol() = 0;
		virtual builder*	getbuilder() { return 0; }
	};
	struct command {
		class builder {
			void			render(const control* parent, const control::command* commands, bool& separator, int& count);
		public:
			builder() = default;
			virtual ~builder() {}
			virtual void	add(const control* parent, const control::command& cmd) = 0;
			virtual void	addseparator() = 0;
			virtual const command* finish() const { return 0; }
			void			render(const control* parent, const control::command* commands);
			virtual void	start() const {}
		};
		const char*			id;
		const char*			name;
		command*			child;
		proci				proc;
		int					image;
		unsigned			key;
		show_s				view;
		explicit operator bool() const { return id != 0; }
		bool				isallow(const control* parent) const;
		constexpr bool		isgroup() const { return id[0] == '*'; }
		const command*		find(const char* id) const;
		const command*		find(unsigned key) const;
	};
	bool					show_border;
	bool					show_background;
	bool					show_toolbar;
	int						splitter;
	static const sprite*	standart_toolbar;
	static const sprite*	standart_tree;
	constexpr control() : show_border(true), show_background(true), show_toolbar(true), splitter(0) {}
	virtual ~control() {}
	virtual void			activating() {}
	void					contextmenu(const command* source);
	void					contextmenu(const command* source, command::builder& builder);
	virtual bool			copy(bool run);
	virtual bool			cut(bool run);
	static command			commands_edit[];
	virtual void			deactivating() {}
	static bool				equal(const char* s1, const char* s2);
	const command*			getcommand(const char* id) const { return getcommands()->find(id); }
	virtual const command*	getcommands() const { return 0; }
	static control*			getfocus();
	virtual const sprite*	getimages() const { return standart_toolbar; }
	virtual const char*		getlabel(stringbuilder& sb) const { return 0; }
	virtual const sprite*	gettreeimages() const { return standart_tree; }
	virtual const char*		geturl(stringbuilder& sb) const { return 0; }
	virtual void			icon(int x, int y, bool disabled, const command& cmd) const;
	virtual bool			isdisabled() const { return false; }
	virtual bool			isfocusable() const { return true; }
	virtual bool			isfocused() const;
	bool					ishilited() const;
	virtual bool			ismodified() const { return false; }
	virtual bool			keyinput(unsigned id); // Default behaivor call shortcut function
	virtual bool			paste(bool run);
	void					postcmd(fncmd proc) const;
	void					postkeyinput(int value) const;
	void					postsetvalue(const char* id, int value) const;
	virtual void			redraw() {}
	virtual void			setfocus(bool instant);
	virtual void			setvalue(const char* id, int value) {}
	int						toolbar(int x, int y, int width, int* next_x = 0) const;
	virtual void			view(const rect& rc);
	virtual void			write(serializer& sr) const {}
};
// Analog of listbox element
struct list : control {
	int						origin, current, origin_width;
	int						lines_per_page, pixels_per_line, pixels_per_width;
	bool					show_grid_lines, show_selection, show_header;
	bool					hilite_odd_lines, drop_shadow;
	rect					current_rect, view_rect;
	static int				current_hilite_row, current_hilite_column, current_hilite_treemark;
	constexpr list() : origin(0), current(0), origin_width(0),
		lines_per_page(0), pixels_per_line(0), pixels_per_width(0),
		show_grid_lines(false), show_selection(true), show_header(true), hilite_odd_lines(true), drop_shadow(false),
		current_rect(), view_rect() {}
	virtual void			collapse(int index) {}
	void					correction();
	void					correction_width();
	virtual void			ensurevisible(); // Ånsure that current selected item was visible on screen if current 'count' is count of items per line
	virtual void			expand(int index) {}
	void					expandall(int max_level);
	int						find(int line, int column, const char* name, int lenght = -1) const;
	int						getblockcount(int index) const;
	virtual int				getcolumn() const { return 0; } // Get current column
	virtual int				getident() const { return 6 * 2 + 4; } // get row ident in levels
	virtual int				getimage(int index) const { return -1; }
	virtual int				getlevel(int index) const { return 0; } // get row ident in levels
	inline int				getline() const { return current; } // get current line
	int						getlinesperpage(int height) const { return height / pixels_per_line; }
	virtual const char*		getname(stringbuilder& sb, int line, int column) const { return 0; }
	int						getnextblock(int index, int increment = 1) const;
	virtual int				getmaximum() const { return 0; }
	virtual int				getmaximumwidth() const { return 0; }
	int						getparent(int index) const;
	int						getroot(int index) const;
	static int				getrowheight(); // Get default row height for any List Control
	virtual int				getwidth(int column) const { return 0; } // Get column width
	void					hilight(const rect& rc, const rect* prc = 0) const;
	virtual bool			isgroup(int index) const { return false; }
	bool					isopen(int index) const;
	bool					keyinput(unsigned id) override;
	void					mousehiliting(const rect& rc, point mouse);
	virtual void			mouseselect(int id, bool pressed);
	void					redraw() override;
	virtual void			row(const rect& rc, int index);
	virtual int				rowheader(const rect& rc) const { return 0; }
	virtual void			rowhilite(const rect& rc, int index) const;
	virtual void			select(int index, int column);
	void					setvalue(const char* id, int v) override;
	void					toggle(int index);
	void					treemark(const rect& rc, int index, int level) const;
	void					view(const rect& rc) override;
};
struct picker : list {
	int						pixels_per_column, elements_per_line;
	constexpr picker() : pixels_per_column(64), elements_per_line(0) {}
	void					ensurevisible();
	bool					keyinput(unsigned id);
	void					mousewheel(unsigned id, point position, int step);
	void					view(const rect& rc) override;
};
struct visual;
struct column {
	const visual*			method;
	const char*				title;
	short					width;
	column_size_s			size;
	total_s					total;
	cflags<column_s>		flags;
	image_flag_s			align;
	array*					source;
	anyreq					path;
	fnlist					plist;
	explicit operator bool() const { return method != 0; }
	int						get(const void* object) const;
	const char*				get(const void* object, stringbuilder& sb) const;
	constexpr bool			is(column_s v) const { return flags.is(v); }
	void					set(const void* object, int v);
	column&					set(image_flag_s v) { align = v; return *this; }
	column&					set(column_size_s v) { size = v; return *this; }
	column&					set(column_s v) { flags.add(v); return *this; }
	column&					set(total_s v) { total = v; return *this; }
	column&					set(array* v) { source = v; return *this; }
	column&					set(const fnlist& v) { plist = v; return *this; }
	column&					setwidth(int v) { width = v; return *this; }
};
struct table : list {
	typedef int				(table::*fncompare)(int i1, int i2, int column) const;
	struct enumi {
		const char*			id;
		const char*			name;
	};
	struct sortparam {
		unsigned short		column;
		short				multiplier; // 1 - ascending, -1 - descending
	};
	vector<column>			columns;
	int						current_column, current_column_maximum, maximum_width;
	bool					no_change_order, no_change_count, read_only, show_totals;
	select_mode_s			select_mode;
	constexpr table() : current_column(0), current_column_maximum(0), maximum_width(0),
		no_change_order(false), no_change_count(false), read_only(false), show_totals(false),
		select_mode(SelectCell) {}
	column&					addcol(const char* name, const anyreq& req, const char* visual_id, array* source = 0);
	column&					addcol(const char* name, const char* visual_id);
	column&					addstdimage();
	virtual void*			addrow() = 0;
	virtual bool			addrow(bool run);
	void					cell(const rect& rc, int line, int column, const char* text);
	void					cellbox(const rect& rc, int line, int column);
	void					celldate(const rect& rc, int line, int column);
	void					celldatetime(const rect& rc, int line, int column);
	void					cellimage(const rect& rc, int line, int column);
	void					cellimagest(const rect& rc, int line, int column);
	void					cellrownumber(const rect& rc, int line, int column);
	void					cellhilite(const rect& rc, int line, int columen, const char* text, image_flag_s aling) const;
	void					cellnumber(const rect& rc, int line, int column);
	void					cellpercent(const rect& rc, int line, int column);
	void					celltext(const rect& rc, int line, int column);
	virtual bool			change(bool run);
	void					changecheck(const rect& rc, int line, int column);
	bool					changefield(const rect& rc, unsigned flags, stringbuilder& sb);
	void					changenumber(const rect& rc, int line, int column);
	void					changetext(const rect& rc, int line, int column);
	void					changeref(const rect& rc, int line, int column);
	static command			commands[];
	static command			commands_add[];
	static command			commands_move[];
	static command			commands_ie[];
	int						comparenm(int i1, int i2, int column) const;
	int						comparest(int i1, int i2, int column) const;
	int						comparer(int i1, int i2, const sortparam* param, int count) const;
	virtual void			clickcolumn(int column) const;
	virtual void			ensurevisible() override;
	bool					isaddable() const { return !no_change_count; }
	bool					ismoveable() const { return !no_change_order; }
	virtual void*			get(int index) const = 0;
	virtual int				getcolumn() const override { return current_column; }
	const command*			getcommands() const override { return commands; }
	void*					getcurrent() const;
	static const char*		getenumname(const void* object, stringbuilder& sb);
	static const char*		getenumid(const void* object, stringbuilder& sb);
	virtual const char*		getheader(stringbuilder& sb, int column) const { return columns[column].title; }
	virtual int				getmaximumwidth() const { return maximum_width; }
	static const char*		getnum8(const void* object, stringbuilder& sb);
	static const char*		getnum16(const void* object, stringbuilder& sb);
	static const char*		getnum32(const void* object, stringbuilder& sb);
	rect					getrect(int row, int column) const;
	virtual const char*		getserialid() const { return 0; }
	int						gettotal(int column) const;
	int						getvalid(int column, int direction = 1) const;
	virtual const visual**	getvisuals() const;
	bool					keyinput(unsigned id) override;
	bool					movedown(bool run);
	bool					moveup(bool run);
	void					mouseselect(int id, bool pressed) override;
	virtual void			remove(int index) = 0;
	virtual bool			removerow(bool);
	virtual void			row(const rect& rc, int index) override; // Draw single row - part of list
	virtual int				rowheader(const rect& rc) const override; // Draw header row
	virtual void			rowtotal(const rect& rc) const; // Draw header row
	void					select(int index, int column = 0) override;
	bool					setting(bool run);
	bool					saveas(bool run);
	void					sort(int i1, int i2, sortparam* ps, int count);
	void					sort(int column, bool ascending);
	bool					sortas(bool run);
	bool					sortds(bool run);
	virtual void			swap(int i1, int i2) = 0;
	void					view(const rect& rc) override;
	static const visual		visuals[];
	void					write(serializer& sr) const override;
	bool					write(const char* url, bool include_header) const;
private:
	column&					addcolimp(const char* name, const anyreq* req, const char* visual_id, array* source, const fnlist* plist);
	void					update_columns(const rect& rc);
};
struct calendar : control {
	datetime				current;
	virtual void			row(const rect& rc, datetime data) const;
	void					view(const rect& rc) override;
};
struct tableref : table, private array {
	struct element {
		void*				object;
	};
	constexpr tableref(unsigned size = sizeof(element)) : array(size) {}
	void*					addref(void* object);
	void*					addrow() override { return 0; };
	void					clear() { array::clear(); }
	int						find(const void* object) const;
	void*					get(int index) const override { return ((element*)array::ptr(index))->object; }
	virtual int				getmaximum() const override { return array::getcount(); }
	void					remove(int index) override { array::remove(index, 1); }
	void					swap(int i1, int i2) override { array::swap(i1, i2); }
	void					sort(int column, bool ascending) { table::sort(column, ascending); }
};
struct tableview : table {
	array&					source;
	constexpr tableview(array& source) : source(source) {}
	void*					addrow() override { return source.addz(); }
	void*					get(int index) const override { return source.ptr(index); }
	int						getmaximum() const { return source.getcount(); }
	void					remove(int index) override { return source.remove(index, 1); }
	void					swap(int i1, int i2) { source.swap(i1, i2); }
};
struct tree : table, protected array {
	struct element {
		unsigned char		level;
		unsigned char		flags;
		unsigned char		type;
		unsigned char		image;
		void				setgroup(bool v) { flags = v ? 1 : 0; }
	};
	bool					sort_rows_by_name;
	constexpr tree(unsigned size = sizeof(element)) : array(size), sort_rows_by_name(false) {}
	void*					addrow() override { return array::add(); }
	void					collapse(int index) override;
	void					clear() { array::clear(); }
	void					expand(int index) override;
	virtual void			expanding(int index, int level) {}
	void*					get(int index) const override { return array::ptr(index); }
	int						getimage(int index) const override;
	int						getlevel(int index) const override;
	virtual int				getmaximum() const override { return array::getcount(); }
	int						gettype(int index) const;
	int						gettreecolumn() const;
	element*				insert(int& index, int level);
	bool					isgroup(int index) const override;
	bool					keyinput(unsigned id) override;
	void					remove(int index) override {}
	void					swap(int i1, int i2) override;
};
// Cell visual drawing
struct visual {
	typedef void			(table::*fnrender)(const rect& rc, int line, int column);
	const char*				id;
	const char*				name;
	image_flag_s			align;
	int						minimal_width, default_width;
	column_size_s			size;
	total_s					total;
	fnrender				render;
	fnrender				change;
	table::fncompare		comparer;
	bool					change_one_click;
	fnlist					plist;
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
	virtual void			redraw(const rect& rc) {}
	virtual void			beforeredraw(const rect& rc) {}
	void					view(const rect& rc) override;
};
class textedit : public scrollable {
	char*					string;
	unsigned				maxlenght;
	int						cashed_width;
	int						cashed_string;
	int						cashed_origin;
	int						p1, p2;
public:
	rect					rctext, rcclient;
	list*					records;
	unsigned				align;
	bool					readonly, update_records, show_records, post_escape;
	//
	textedit(char* string, unsigned maximum_lenght, bool select_text);
	//
	void					clear();
	virtual void			cashing(rect rc);
	bool					copy(bool run) override;
	void					correct();
	bool					cut(bool run) override;
	bool					editing(rect rc);
	void					ensurevisible(int linenumber);
	int						getbegin() const;
	command*				getcommands() const override;
	int						getend() const;
	point					getpos(rect rc, int index, unsigned state) const;
	int						getrecordsheight() const;
	int						hittest(rect rc, point pt, unsigned state) const;
	void					invalidate() override;
	bool					isshowrecords() const;
	bool					keyinput(unsigned id) override;
	int						lineb(int index) const;
	int						linee(int index) const;
	int						linen(int index) const;
	bool					left(bool shift, bool ctrl);
	bool					paste(bool run) override;
	void					paste(const char* string);
	void					redraw(const rect& rc) override;
	bool					right(bool shift, bool ctrl);
	bool					select(int index, bool shift);
	unsigned				select_all(bool run);
	void					setcount(int value) {}
	void					setrecordlist(const char* string);
	void					setvalue(const char* id, int v) override;
	void					updaterecords(bool setfilter);
};
class form {
	rect					rc;
	int						title;
public:
	virtual void			before() {}
	int						edit();
	void					field(const char* title, const anyval& ev);
	void					field(control& ev);
	virtual bool			keyinput(unsigned id) {}
	virtual void			view() {}
	constexpr form() : rc{}, title(100) {}
};
void						activate(control* p);
void						close(control* p);
bool						edit(control& e, fnevent heatbeat = 0);
control*					getactivated();
const char*					getlabel(const void* object, stringbuilder& sb);
control*					openurl(const char* url);
}
void						addelement(const rect& rc, const anyval& value);
void						application(fnevent heartproc, shortcut* shortcuts);
void						application(const char* name, fnevent showproc, fnevent heartproc, shortcut* shortcuts);
inline void					application() { application(0, 0); }
void						application_initialize();
int							button(int x, int y, int width, const anyval& value, bool& result, const char* label, const char* tips = 0, int key = 0);
int							button(int x, int y, int width, fnevent proc, const char* label, const char* tips = 0, int key = 0);
bool						buttonr(int & x, int y, fnevent proc, const char* label, void* object, int key);;
int							checkbox(int x, int y, int width, const anyval& value, const char* label, const char* tips = 0);
void						dockbar(rect& rc);
bool						dropdown(const rect& rc, controls::control& e);
int							field(int x, int y, int width, const char* label, color& value, int header_width, const char* tips = 0);
int							field(int x, int y, int width, const char* label, const char*& sev, int header_width, fnchoose choose_proc = 0);
int							field(int x, int y, int width, const char* header_label, char* sev, unsigned size, int header_width, fnchoose choose_proc = 0);
int							field(int x, int y, int width, const char* label, const anyval& av, int header_width, int digits);
int							field(int x, int y, int width, const char* label, const anyval& av, int header_width, const array& source, const void* object, const fnlist& plist, const char* tips = 0);
void						fieldf(const rect& rc, unsigned flags, const anyval& ev, int digits, bool increment, bool istext, fnchoose choose_proc);
void						fieldm(const rect& rc, const anyval& av, const array& source, const void* object, const fnlist& plist, bool instant);
void						fieldc(const rect& rc, const anyval& av, const array& source, const void* object, const fnlist& plist, const char* tips);
bool						isfocused(const anyval& value);
bool						isfocused(const rect& rc, const anyval& value);
int							radio(int x, int y, int width, const anyval& value, const char* label, const char* tips = 0);
void						setfocus(const anyval& value, bool instant = false);
void						setposition(int& x, int& y, int& width, int padding = -1);
void						titletext(int& x, int y, int& width, unsigned flags, const char* label, int title, const char* separator = 0);
}