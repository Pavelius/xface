#include "bsdata.h"
#include "collection.h"
#include "crt.h"
#include "draw.h"
#include "draw_control.h"
#include "draw_grid.h"
#include "io_plugin.h"
#include "settings.h"

using namespace	draw;

struct application_window {
	int						x, y, width, height;
	int						header_width;
};

extern bool					sys_optimize_mouse_move;
bool						metrics::show::padding;
bool						metrics::show::statusbar;
static int					current_tab;
static settings*			current_header;
static controls::control*	active_workspace_tab;
static application_window	window = {0, 0, 0, 0, 160};
static const char*			settings_file_name = "settings.json";
static rect					current_rect;

aref<controls::control*>	getdocked(aref<controls::control*> result, dock_s type);

static struct dock {
	const char*	name;
	const char*	id;
} dock_data[DockWorkspace + 1] = {{"Присоединить слева", "dock_left"},
{"Присоединить слева и снизу", "dock_left_bottom"},
{"Присоединить справа", "dock_right"},
{"Присоединить справа и снизу", "dock_right_bottom"},
{"Присоединить снизу", "dock_bottom"},
{"На рабочем столе", "dock_workspace"}
};
getstr_enum(dock)
bsreq dock_type[] = {
	BSREQ(dock, id, text_type),
	BSREQ(dock, name, text_type),
{0}
};
BSMETA(dock)
bsreq draw::controls::control::plugin::metadata[] = {
	BSREQ(plugin, id, text_type),
	BSREQ(plugin, dock, dock_type),
{}};

static const char* get_setting_name(char* result, const char* result_maximum, void* p) {
	return szprint(result, result_maximum, ((settings*)p)->name);
}

static void callback_settab() {
	current_tab = hot.param;
}

static int compare_settings(const void* p1, const void* p2) {
	const settings* e1 = *((settings**)p1);
	const settings* e2 = *((settings**)p2);
	if(e1->priority != e2->priority)
		return e1->priority - e2->priority;
	return strcmp(e1->name, e2->name);
}

static void getsiblings(settings** result, unsigned maximum_count, settings* parent) {
	settings** ps = result;
	settings** pe = result + maximum_count - 1;
	settings* tabs = parent->child();
	if(tabs) {
		for(settings* p = tabs; p; p = p->next) {
			if(p->e_visible && !p->e_visible(*p))
				continue;
			if(ps < pe)
				*ps++ = p;
		}
	}
	*ps = 0;
	qsort(result, zlen(result), sizeof(result[0]), compare_settings);
}

static void callback_button() {
	auto p = (settings*)hot.param;
	if(p->e_execute)
		p->e_execute();
}

static void callback_bool() {
	auto p = (settings*)hot.param;
	*((bool*)p->data) = !*((bool*)p->data);
}

static void callback_radio() {
	auto p = (settings*)hot.param;
	*((int*)p->data) = p->value;
}

static void callback_up() {
	auto p = (settings*)hot.param;
	(*((int*)p->data))--;
}

static void callback_down() {
	auto p = (settings*)hot.param;
	(*((int*)p->data))++;
}

static void callback_choose_folder(const storage& ev) {
	char temp[260];
	ev.getf(temp, zendof(temp));
	if(draw::dialog::folder("Укажите папку", temp))
		ev.set(temp);
}

static void callback_choose_color() {
	auto p = (color*)hot.param;
	draw::dialog::color(*p);
}

static void callback_edit() {
	char temp[4196]; temp[0] = 0;
	auto p = (settings*)hot.param;
	switch(p->type) {
	case settings::TextPtr:
	case settings::UrlFolderPtr:
		if(*((const char**)p->data))
			zcpy(temp, *((const char**)p->data), sizeof(temp) - 1);
		break;
	case settings::Int:
		sznum(temp, *((int*)p->data));
		break;
	}
	auto push_focus = getfocus();
	controls::textedit te(temp, sizeof(temp), true);
	setfocus((int)&te, true);
	if(te.editing(current_rect)) {
		switch(p->type) {
		case settings::TextPtr:
		case settings::UrlFolderPtr:
			if(temp[0])
				*((const char**)p->data) = szdup(temp);
			else
				*((const char**)p->data) = 0;
			break;
		case settings::Int:
			*((int*)p->data) = sz2num(temp);
			break;
		}
	}
	setfocus(push_focus, true);
}

static struct widget_settings_header : controls::list {
	settings*	rows[128];
	int			maximum;

	void initialize() {
		getsiblings(rows, sizeof(rows) / sizeof(rows[0]), &settings::root);
		maximum = zlen(rows);
		if(current >= maximum - 1)
			current = maximum;
		if(current < 0)
			current = 0;
	}

	void row(const rect& rc, int index) override {
		list::row({rc.x1 + 1, rc.y1 + 1, rc.x2 - 1, rc.y2}, index);
		textc(rc.x1 + 4, rc.y1 + 4, rc.width() - 8, rows[index]->name);
	}

	settings* getcurrent() {
		return rows[current];
	}

	int getmaximum() const override {
		return maximum;
	}

} setting_header;

static struct widget_control_viewer : controls::gridref {

	void initialize() {
		no_change_order = true;
		no_change_count = true;
		addcol("name", "Наименование", "text");
		addcol("dock", "Расположение", "ref");
		for(auto p = plugin::first; p; p = p->next)
			add(p);
	}

	const char*	getname(char* result, const char* result_maximum, int row, int column) const override {
		auto p = (control::plugin*)get(row);
		if(strcmp(columns[column].id, "name") == 0)
			return p->getcontrol().getlabel(result, result_maximum);
		return gridref::getname(result, result_maximum, row, column);
	}

	widget_control_viewer() : gridref(draw::controls::control::plugin::metadata) {
	}

} control_viewer;

int draw::field(int x, int y, int width, unsigned flags, color& value, const char* header_label, const char* tips, int header_width) {
	draw::state push;
	setposition(x, y, width);
	decortext(flags);
	if(header_label && header_label[0])
		titletext(x, y, width, flags, header_label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	char temp[128]; szprint(temp, zendof(temp), "%1i, %2i, %3i", value.r, value.g, value.b);
	focusing((int)&value, flags, rc);
	if(buttonh(rc,
		ischecked(flags), (flags&Focused) != 0, (flags&Disabled) != 0, true, value,
		temp, KeyEnter, false, tips))
		execute(callback_choose_color, (int)&value);
	return rc.height() + metrics::padding * 2;
}

static struct widget_settings : controls::control {

	static const char* getname(char* temp, settings& e) {
		zcpy(temp, e.name);
		szupper(temp);
		return temp;
	}

	const char* getlabel(char* result, const char* result_maximum) const override {
		return szprint(result, result_maximum, "Настройки");
	}

	static int element(int x, int y, int width, unsigned flags, settings& e) {
		struct cmdv : runable {
			settings*		pe;
			draw::callback	p;
			int	getid() const { return (int)pe; }
			void execute() const { draw::execute(p, (int)pe); }
			void set(draw::callback p, settings& e) { this->p = p; pe = &e; }
		};
		const auto title = 160;
		settings* pc;
		char temp[512]; temp[0] = 0;
		if(e.e_visible && !e.e_visible(e))
			return 0;
		int y1 = y;
		cmdv ec;
		switch(e.type) {
		case settings::Radio:
			ec.set(callback_radio, e);
			y += radio(x, y, width, flags | ((*((int*)e.data) == e.value) ? Checked : 0), ec, getname(temp, e), 0);
			break;
		case settings::Bool:
			ec.set(callback_bool, e);
			y += metrics::padding;
			y += checkbox(x, y, width, flags | (*((bool*)e.data) ? Checked : 0), ec, getname(temp, e), 0);
			break;
		case settings::Int:
			// Есть максимум
			if(e.value) {
				auto w = (getdigitscount(e.value) + 1)*textw("0") + metrics::padding * 2 + 19;
				if(title + w < width)
					width = title + w;
			}
			y += field(x, y, width, flags, *((int*)e.data), e.name, 0, title);
			break;
		case settings::Color:
			y += field(x, y, width, flags, *((color*)e.data), e.name, 0, title);
			break;
		case settings::Button:
			ec.set(callback_button, e);
			y += button(x, y, width, flags, ec, getname(temp, e), 0);
			break;
		case settings::TextPtr:
			y += field(x, y, width, flags, *((const char**)e.data), e.name, 0, title);
			break;
		case settings::UrlFolderPtr:
			y += field(x, y, width, flags, *((const char**)e.data), e.name, 0, title, callback_choose_folder);
			break;
		case settings::Control:
			break;
		case settings::Group:
			pc = e.child();
			if(!pc)
				return 0;
			if(true) {
				auto x1 = x;
				auto w1 = width;
				auto y2 = y;
				auto height = draw::texth() + metrics::padding * 2;
				y += height;
				if(e.name) {
					x1 += metrics::padding;
					w1 -= metrics::padding * 2;
				}
				for(; pc; pc = pc->next)
					y += element(x1, y, w1, flags, *pc);
				if(e.name) {
					color c1 = colors::border.mix(colors::window, 128);
					color c2 = c1.darken();
					gradv({x, y2, x + width, y2 + height}, c1, c2);
					fore = colors::text.mix(c1, 196);
					text(x + (width - textw(e.name)) / 2, y2 + metrics::padding, e.name);
					rectb({x, y2, x + width, y + metrics::padding}, colors::border);
				}
				y += metrics::padding * 2;
			}
			break;
		}
		return y - y1;
	}

	void view(const rect& rcorigin) override {
		auto rc = rcorigin;
		draw::state push;
		settings* tabs[128];
		fore = colors::text;
		splitv(rc.x1, rc.y1, window.header_width, rc.height(), 6, 64, 282, false);
		setting_header.show_border = metrics::show::padding;
		setting_header.view({rc.x1, rc.y1, rc.x1 + window.header_width, rc.y2});
		rc.x1 += window.header_width + 6;
		auto top = setting_header.getcurrent();
		// При изменении текущего заголовка
		if(top != current_header) {
			current_header = top;
			current_tab = -1;
		}
		if(!top)
			return;
		getsiblings(tabs, sizeof(tabs) / sizeof(tabs[0]), top);
		if(tabs[0]) {
			// Покажем дополнительную панель
			if(current_tab == -1)
				current_tab = 0;
			int h1 = 28;
			// Нарисуем закладки
			auto hilited = -1;
			if(draw::tabs({rc.x1, rc.y1, rc.x2, rc.y1 + h1 + 1}, false, false,
				(void**)tabs, 0, zlen(tabs), current_tab, &hilited,
				get_setting_name)) {
				draw::execute(callback_settab);
				hot.param = hilited;
			}
			line(rc.x1, rc.y1 + h1, rc.x2, rc.y1 + h1, colors::border);
			rc.y1 += h1 + metrics::padding * 2;
			rc.x1 += metrics::padding;
			rc.x2 -= metrics::padding;
			// Нариуем текущую закладку
			if(current_tab != -1) {
				int w4 = rc.width();
				int w3 = imin(w4, 640);
				auto p1 = tabs[current_tab];
				switch(p1->type) {
				case settings::Control:
					((control*)p1->data)->view(rc);
					break;
				default:
					for(auto p = p1->child(); p; p = p->next)
						rc.y1 += element(rc.x1, rc.y1, w3, 0, *p);
					break;
				}
			}
		}
	}

	widget_settings() {
		show_background = false;
		show_border = false;
	}

} widget_settings_control;

static const char* get_control_name(char* result, const char* result_maximum, void* object) {
	return ((controls::control*)object)->getlabel(result, result_maximum);
}

static struct widget_application : draw::controls::control {

	control*		hotcontrols[48];

	const char* getlabel(char* result, const char* result_maximum) const override {
		return szprint(result, result_maximum, "Главный");
	}

	const command* getcommands() const override {
		return 0;
	}

	static int find(control** source, control* value) {
		for(auto p = source; *p; p++) {
			if(p[0] == value)
				return p - source;
		}
		return -1;
	}

	static void workspace(rect rc, bool allow_multiply_window) {
		control* p1[64];
		auto c1 = getdocked(p1, DockWorkspace);
		//int c2 = c1 + dialog::select(p1 + c1);
		if(c1.count == 1 && !allow_multiply_window) {
			active_workspace_tab = p1[0];
			active_workspace_tab->view(rc);
		} else if(c1) {
			auto last_active_workspace_tab = active_workspace_tab;
			auto current_select = find(p1, active_workspace_tab);
			if(current_select == -1)
				current_select = 0;
			active_workspace_tab = p1[current_select];
			auto ec = active_workspace_tab;
			const int dy = draw::texth() + 8;
			rect rct = {rc.x1, rc.y1, rc.x2, rc.y1 + dy};
			auto result = draw::tabs(rct, false, false, (void**)p1, 0, c1.count,
				current_select, &current_select,
				get_control_name,
				{2, 0, 2, 0});
			//		if(c2 > c1)
			//		{
			//			rct.x1 += draw::tabs(rct, TabsControl, HideBackground, (void**)p1, c1, c2,
			//				current_select, &current_select, get_control_name, get_control_info, 2);
			//		}
			//		if(getcommand() == TabsControl)
			//		{
			//			if(current_select != -1)
			//				active_workspace_tab = p1[current_select];
			//		}
			//		else if(getcommand() == TabsCloseControl)
			//		{
			//			if(current_select != -1)
			//				close_workspace_tab = p1[current_select];
			//		}
			//		if(active_workspace_tab != last_active_workspace_tab)
			//		{
			//			if(active_workspace_tab)
			//				active_workspace_tab->execute(Update, true);
			//		}
			rc.y1 += dy;
			unsigned flags = ec->isfocused() ? Focused : 0;
			if(ec->isdisabled())
				flags |= Disabled;
			draw::focusing((int)ec, flags, rc);
			ec->view(rc);
		}
	}

	void view(const rect& rc) override {
		dockbar(rc);
		workspace(rc, true);
	}

	widget_application() {
		show_background = false;
		show_border = false;
		memset(hotcontrols, 0, sizeof(hotcontrols));
	}

} widget_application_control;

static void setting_appearance_general_metrics() {
	settings& e1 = settings::root.gr("Рабочий стол").gr("Общие").gr("Метрика");
	e1.add("Отступы", metrics::padding);
	e1.add("Ширина скролла", metrics::scroll);
}

void set_light_theme();
void set_dark_theme();

static void setting_appearance_forms() {
	settings& e2 = settings::root.gr("Цвета").gr("Общие");
	e2.add("Установить светлую тему", set_light_theme);
	e2.add("Установить темную тему", set_dark_theme);
	settings& e3 = settings::root.gr("Цвета").gr("Формы");
	e3.add("Цвет текста", colors::text);
	e3.add("Цвет окна", colors::window);
	e3.add("Цвет формы", colors::form);
	e3.add("Цвет границы", colors::border);
	e3.add("Активный цвет", colors::active);
	e3.add("Цвет кнопки", colors::button);
	e3.add("Цвет редактирования", colors::edit);
	e3.add("Цвет закладок", colors::tabs::back);
	e3.add("Цвет текста закладок", colors::tabs::text);
	e3.add("Цвет подсказки", colors::tips::back);
	e3.add("Цвет текста подсказки", colors::tips::text);
}

static void setting_appearance_general_view() {
	settings& e1 = settings::root.gr("Рабочий стол").gr("Общие").gr("Вид");
	e1.add("Показывать панель статуса", metrics::show::statusbar);
	e1.add("Показывать левую панель элементов", metrics::show::left);
	e1.add("Показывать правую панель элементов", metrics::show::right);
	e1.add("Показывать нижнюю панель элементов", metrics::show::bottom);
	e1.add("Отступы на главном окне", metrics::show::padding);
	e1.add("Использовать оптимизацию при движении мишки", sys_optimize_mouse_move);
}

static void setting_appearance_controls() {
	control_viewer.initialize();
	if(!control_viewer.getcount())
		return;
	settings& e1 = settings::root.gr("Расширения").add("Элементы", control_viewer);
}

static struct application_plugin : draw::initplugin {

	void initialize() override {
		setting_appearance_general_metrics();
		setting_appearance_forms();
		setting_appearance_general_view();
		setting_appearance_controls();
		// Make header
		setting_header.initialize();
	}

	static void exit_application() {
		point pos, size;
		getwindowpos(pos, size);
		window.x = pos.x;
		window.y = pos.y;
		window.width = size.x;
		window.height = size.y;
		io::write(settings_file_name, "settings", 0);
	}

	void after_initialize() override {
		atexit(exit_application);
		io::read(settings_file_name, "settings", 0);
	}

	application_plugin() : initplugin(3) {}

} application_plugin_instance;

static controls::control* layouts[] = {&widget_application_control, &widget_settings_control};

static void get_control_status(controls::control* object) {
	char temp[260];
	draw::statusbar("Переключить вид на '%1'", object->getlabel(temp, zendof(temp)));
}

void draw::application() {
	auto current_tab = 0;
	while(ismodal()) {
		auto pc = layouts[current_tab];
		auto tb = pc->getimages();
		auto commands = pc->getcommands();
		draw::fore = colors::text;
		rect rc = {0, 0, draw::getwidth(), draw::getheight()};
		draw::rectf(rc, colors::form);
		if(metrics::show::statusbar)
			rc.y2 -= draw::statusbardw();
		rect rt = rc;
		rt.y2 = rt.y1 + (tb ? (tb->get(0).sy + 4 * 2) : 24);
		sheetline(rt, true);
		rc.y1 += rt.height();
		rt.x1 += 2; rt.y1 += 1; rt.y2 -= 3; rt.x2 -= 2;
		if(metrics::show::padding)
			rc.offset(metrics::padding);
		pc->toolbar(rt.x1, rt.y1, rt.width());
		pc->view(rc);
		auto hilite_tab = -1;
		auto reaction = draw::tabs(rt, false, true, (void**)layouts, 0,
			sizeof(layouts) / sizeof(layouts[0]), current_tab, &hilite_tab,
			get_control_name,
			{0, metrics::padding, 0, metrics::padding});
		if(hilite_tab != -1)
			get_control_status(layouts[hilite_tab]);
		domodal();
		if(reaction == 1)
			current_tab = hilite_tab;
		switch(hot.key) {
		case F2: metrics::show::bottom = !metrics::show::bottom; break;
		case Alt + F2: metrics::show::left = !metrics::show::left; break;
		case Ctrl + F2: metrics::show::right = !metrics::show::right; break;
		}
	}
}

void set_dark_theme();
void set_light_theme();

void draw::application_initialize() {
	set_light_theme();
	initialize();
	for(auto p = controls::control::plugin::first; p; p = p->next)
		p->after_initialize();
	create(window.x, window.y, window.width, window.height, WFResize | WFMinmax, 32);
}

static struct settings_settings_strategy : io::strategy {

	void write(io::writer& file, settings& e) {
		switch(e.type) {
		case settings::Group:
			file.open(e.name);
			for(settings* p = (settings*)e.data; p; p = p->next)
				write(file, *p);
			file.close(e.name);
			break;
		case settings::Bool:
			file.set(e.name, *((bool*)e.data));
			break;
		case settings::Int:
		case settings::Color:
			file.set(e.name, *((int*)e.data));
			break;
		case settings::Radio:
			if(*((int*)e.data) == e.value)
				file.set(e.name, 1);
			break;
		case settings::TextPtr:
		case settings::UrlFolderPtr:
			file.set(e.name, *((const char**)e.data));
			break;
		}
	}

	void write(io::writer& file, void* param) override {
		write(file, settings::root);
	}

	settings* find(io::node& n) {
		auto result = (settings*)settings::root.data;
		if(n.parent && strcmp(n.parent->name, "Root") != 0) {
			result = find(*n.parent);
			if(!result)
				return 0;
			if(result->type != settings::Group)
				return 0;
			result = (settings*)result->data;
		}
		return result->find(szdup(n.name));
	}

	int getnum(const char* value) const {
		return sz2num(value);
	}

	void set(io::node& n, const char* value) override {
		auto e = find(n);
		if(!e)
			return;
		switch(e->type) {
		case settings::Int:
		case settings::Color:
			*((int*)e->data) = getnum(value);
			break;
		case settings::Bool:
			*((bool*)e->data) = (getnum(value) ? true : false);
			break;
		case settings::Radio:
			*((int*)e->data) = e->value;
			break;
		case settings::TextPtr:
		case settings::UrlFolderPtr:
			*((const char**)e->data) = szdup(value);
			break;
		}
	}

	settings_settings_strategy() : strategy("settings", "settings") {}

} settings_settings_strategy_instance;

static struct window_settings_strategy : io::strategy {

	void write(io::writer& file, void* param) override {
		file.set("x", window.x);
		file.set("y", window.y);
		file.set("width", window.width);
		file.set("height", window.height);
		file.set("header_width", window.header_width);
	}

	void set(io::node& n, const char* value) override {
		if(n == "x")
			window.x = sz2num(value);
		else if(n=="y")
			window.y = sz2num(value);
		else if(n == "width")
			window.width = sz2num(value);
		else if(n == "height")
			window.height = sz2num(value);
		else if(n == "header_width")
			window.header_width = sz2num(value);
	}

	window_settings_strategy() : strategy("window", "settings") {}

} window_settings_strategy_instance;

static struct controls_settings_strategy : io::strategy {

	void write(io::writer& file, void* param) override {
		for(auto pp = controls::control::plugin::first; pp; pp = pp->next) {
			auto& e = pp->getcontrol();
			auto id = pp->id;
			if(!id || id[0] == 0)
				continue;
			file.open(id);
			file.set("Docking", pp->dock);
			file.close(id);
		}
	}

	settings* find(io::node& n) {
		settings* result = (settings*)settings::root.data;
		if(n.parent && szcmpi(n.parent->name, "Root") != 0) {
			result = find(*n.parent);
			if(!result)
				return 0;
			if(result->type != settings::Group)
				return 0;
			result = (settings*)result->data;
		}
		return result->find(szdup(n.name));
	}

	bool istrue(const char* value) const {
		return value[0] == 't'
			&& value[1] == 'r'
			&& value[2] == 'u'
			&& value[3] == 'e'
			&& value[4] == 0;
	}

	void set(io::node& n, const char* value) override {
		if(!n.parent || !n.parent->parent)
			return;
		auto e = const_cast<controls::control::plugin*>(controls::control::plugin::find(n.parent->name));
		if(!e)
			return;
		else if(szcmpi(n.name, "Docking") == 0)
			e->dock = (dock_s)sz2num(value);
	}

	controls_settings_strategy() : strategy("controls", "settings") {}

} controls_settings_strategy_instance;