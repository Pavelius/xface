#include "bsreq.h"
#include "crt.h"
#include "draw.h"
#include "draw_control.h"
#include "io_plugin.h"
#include "setting.h"

using namespace	setting;
using namespace	draw;
using namespace	draw::controls;

struct application_window {
	int						x, y, width, height;
	int						header_width;
	unsigned				flags;
};

extern bool					sys_optimize_mouse_move;
bool						metrics::show::padding;
bool						metrics::show::statusbar;
static bool					use_short_name_label;
static bool					use_no_extension_label;
static bool					use_uppercase_label;
static const header*		current_header;
static int					current_tab;
static char					last_open_file[260];
static controls::control*	current_active_control;
static application_window	window = {0, 0, 0, 0, 160, WFMinmax | WFResize};
static const char*			settings_file_name = "settings.json";
static vector<controls::control*> active_controls;

aref<controls::control*> getdocked(control** result, unsigned count, dock_s type);

BSDATA(docki) = {{"dock_left", "Присоединить слева"},
{"dock_left_bottom", "Присоединить слева и снизу"},
{"dock_right", "Присоединить справа"},
{"dock_right_bottom", "Присоединить справа и снизу"},
{"dock_bottom", "Присоединить снизу"},
{"dock_workspace", "На рабочем столе"},
};

struct streca {
	const element*	e;
	const header*	h;
};

static int getpriority(const header* p) {
	if(strcmp(p->page, "Общие") == 0)
		return 1;
	return 10;
}

static int compare(const char* s1, const char* s2) {
	if(s1 == s2)
		return 0;
	if(!s1)
		return -1;
	if(!s2)
		return 1;
	return strcmp(s1, s2);
}

static int compare(const header* h1, const header* h2) {
	auto c1 = compare(h1->division, h2->division);
	if(c1)
		return c1;
	auto r1 = getpriority(h1);
	auto r2 = getpriority(h2);
	if(r1 != r2)
		return r1 - r2;
	auto c2 = compare(h1->page, h2->page);
	if(c2)
		return c2;
	return compare(h1->group, h2->group);
}

class stelemsa : public adat<streca, 128> {
	static int compare_elements(const void* p1, const void* p2) {
		auto e1 = (streca*)p1;
		auto e2 = (streca*)p2;
		auto c1 = compare(e1->h, e2->h);
		if(c1)
			return c1;
		return strcmp(e1->e->name, e2->e->name);
	}
public:
	void select() {
		for(auto pm = header::first; pm; pm = pm->next) {
			for(auto& e : pm->elements) {
				auto p = add();
				p->e = &e;
				p->h = pm;
			}
		}
	}
	void match_filled() {
		auto ps = data;
		for(auto& e : *this) {
			if(e.e->var.iszero())
				continue;
			*ps++ = e;
		}
		count = ps - data;
	}
	void match_visible() {
		auto ps = data;
		for(auto& e : *this) {
			if(e.e->test && !e.e->test())
				continue;
			if(e.h->visible && !e.h->visible())
				continue;
			*ps++ = e;
		}
		count = ps - data;
	}
	void sort() {
		qsort(data, count, sizeof(data[0]), compare_elements);
	}
};

class strowsa : public adat<const header*, 128> {
	static int compare_elements(const void* p1, const void* p2) {
		return compare(*((header**)p1), *((header**)p2));
	}
	bool hasdivision(const header* p) const {
		for(auto pe : *this) {
			if(strcmp(p->division, pe->division) == 0)
				return true;
		}
		return false;
	}
	bool haspage(const header* p) const {
		for(auto pe : *this) {
			if(strcmp(p->page, pe->page) == 0)
				return true;
		}
		return false;
	}
public:
	void sort() {
		if(count)
			qsort(data, count, sizeof(data[0]), compare_elements);
	}
	void select_divisions() {
		for(auto pm = header::first; pm; pm = pm->next) {
			if(hasdivision(pm))
				continue;
			add(pm);
		}
	}
	void select_pages(const header* parent) {
		for(auto pm = header::first; pm; pm = pm->next) {
			if(strcmp(parent->division, pm->division) != 0)
				continue;
			if(haspage(pm))
				continue;
			add(pm);
		}
	}
	void select_groups(const header* parent) {
		for(auto pm = header::first; pm; pm = pm->next) {
			if(strcmp(parent->division, pm->division) != 0)
				continue;
			if(strcmp(parent->page, pm->page) != 0)
				continue;
			add(pm);
		}
	}
	void select_divisions(const stelemsa& source) {
		for(auto& e : source) {
			if(hasdivision(e.h))
				continue;
			add(e.h);
		}
	}
	void select_pages(const stelemsa& source, const header* parent) {
		for(auto& e : source) {
			if(strcmp(parent->division, e.h->division) != 0)
				continue;
			if(haspage(e.h))
				continue;
			add(e.h);
		}
	}
	void select_groups(const stelemsa& source, const header* parent) {
		for(auto& e : source) {
			if(strcmp(parent->division, e.h->division) != 0)
				continue;
			if(strcmp(parent->page, e.h->page) != 0)
				continue;
			if(haspage(e.h))
				continue;
			add(e.h);
		}
	}
};

static const char* get_page_name(const void* p, stringbuilder& sb) {
	return ((header*)p)->page;
}

static void callback_settab() {
	current_tab = hot.param;
}

static void callback_button() {
	auto p = (element*)hot.param;
	if(p->var.type == Button && p->var.data)
		((pcall)p->var.data)();
}

static bool choose_folder(const void* object, array& source, void* pointer) {
	auto p = (const char**)pointer;
	char temp[260]; memset(temp, 0, sizeof(temp));
	if(*p)
		zcpy(temp, *p, sizeof(temp) - 1);
	if(!draw::dialog::folder("Укажите папку", temp))
		return false;
	*p = szdup(temp);
	return true;
}

static void callback_choose_color() {
	auto p = (color*)hot.param;
	draw::dialog::color(*p);
}

static const char* getname(char* temp, const setting::element& e) {
	zcpy(temp, e.name);
	szupper(temp);
	return temp;
}

static int render_element(int x, int y, int width, unsigned flags, const setting::element& e) {
	const auto title = 160;
	char temp[512]; temp[0] = 0;
	if(e.test && !e.test())
		return 0;
	auto y1 = y;
	int d;
	switch(e.var.type) {
	case setting::Radio:
		y += radio(x, y, width, anyval(e.var.data, sizeof(int), e.var.value), getname(temp, e), 0);
		break;
	case setting::Bool:
		y += metrics::padding;
		y += checkbox(x, y, width, anyval(e.var.data, sizeof(bool), 1), getname(temp, e), 0);
		break;
	case setting::Number:
		d = 0;
		if(e.param) {
			d = e.param;
			auto w = (d + 1)*textw("0") + metrics::padding * 2 + 19 + 4;
			if(title + w < width)
				width = title + w;
		}
		y += field(x, y, width, e.name, anyval(e.var.data, e.var.size, e.var.value), title, d);
		break;
	case setting::Color:
		y += field(x, y, width, e.name, *((color*)e.var.data), title);
		break;
	case setting::Button:
		if(true) {
			auto result = false;
			y += button(x, y, width, e, result, getname(temp, e), 0);
			if(result)
				draw::execute(callback_button, (int)&e);
		}
		break;
	case setting::Text:
		y += field(x, y, width, e.name, *((const char**)e.var.data), title);
		break;
	case setting::Url:
		y += field(x, y, width, e.name, *((const char**)e.var.data), title, choose_folder);
		break;
	case setting::Control:
		break;
	}
	return y - y1;
}

static int render_element(int x, int y, int width, unsigned flags, const setting::header& e) {
	auto x1 = x, y2 = y, w1 = width;
	auto height = draw::texth() + metrics::padding * 2;
	if(e.group) {
		y += height;
		x1 += metrics::padding;
		w1 -= metrics::padding * 2;
	}
	auto yc = y;
	for(auto& pe : e.elements)
		y += render_element(x1, y, w1, flags, pe);
	if(y == yc)
		return 0;
	if(e.group) {
		color c1 = colors::border.mix(colors::window, 128);
		color c2 = c1.darken();
		gradv({x, y2, x + width, y2 + height}, c1, c2);
		fore = colors::text.mix(c1, 196);
		text(x + (width - textw(e.group)) / 2, y2 + metrics::padding, e.group);
		rectb({x, y2, x + width, y + metrics::padding}, colors::border);
		y += metrics::padding * 2;
	}
	return y - y2;
}

static struct widget_settings_header : controls::list {
	strowsa rows;
	void initialize() {
		rows.clear();
		rows.select_divisions();
		rows.sort();
		if(current >= getmaximum() - 1)
			current = getmaximum();
		if(current < 0)
			current = 0;
	}
	void row(const rect& rc, int index) override {
		list::row({rc.x1 + 1, rc.y1 + 1, rc.x2 - 1, rc.y2}, index);
		textc(rc.x1 + metrics::edit.x1, rc.y1 + metrics::edit.y1, rc.width() + metrics::edit.width(), rows[index]->division);
	}
	const header* getcurrent() {
		return rows[current];
	}
	int getmaximum() const override {
		return rows.getcount();
	}
} setting_header;

static struct widget_control_viewer : controls::tableref {
	void initialize() {
		no_change_order = true;
		no_change_count = true;
		addcol("Наименование", ANREQ(plugin, id), "text").set(ColumnReadOnly);
		addcol("Расположение", ANREQ(plugin, dock), "enum");
		//addcol(type, "visible", "Видимость", "checkbox");
		for(auto p = plugin::first; p; p = p->next) {
			auto pc = p->getcontrol();
			if(!pc)
				continue;
			addref(p);
		}
	}
	void* addrow() override { return 0; }
	void remove(int index) override {}
} control_viewer;

int draw::field(int x, int y, int width, const char* label, color& value, int header_width, const char* tips) {
	draw::state push;
	setposition(x, y, width);
	decortext(0);
	if(label && label[0])
		titletext(x, y, width, 0, label, header_width);
	rect rc = {x, y, x + width, y + draw::texth() + 8};
	char temp[128]; stringbuilder sb(temp);
	sb.add("%1i, %2i, %3i", value.r, value.g, value.b);
	anyval av(value, 0);
	auto focused = isfocused(rc, av);
	if(buttonh(rc,
		false, focused, false, true, value,
		temp, KeyEnter, false, tips))
		execute(callback_choose_color, (int)&value);
	return rc.height() + metrics::padding * 2;
}

const char* draw::controls::getlabel(const void* object, stringbuilder& sb) {
	auto pc = (controls::control*)object;
	auto p = pc->getlabel(sb);
	if(p != sb.begin())
		sb.add(p);
	char temp[260]; stringbuilder sb1(temp);
	p = pc->geturl(sb1);
	if(p) {
		if(use_short_name_label)
			sb.add(szfname(p));
		else
			sb.add(p);
		if(use_no_extension_label) {
			char* p1 = (char*)szext(p);
			if(p1) {
				while(p1 > sb.begin() && p1[-1] == '.')
					p1--;
				sb.set(p1);
			}
		}
	}
	if(pc->ismodified())
		sb.add("*");
	if(use_uppercase_label)
		szupper(sb, 1);
	return sb;
}

static struct widget_settings : controls::control {
	const char* getlabel(stringbuilder& sb) const override {
		return "Настройки";
	}
	void view(const rect& rcorigin) override {
		auto rc = rcorigin;
		draw::state push;
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
		strowsa tabs;
		tabs.select_pages(top);
		if(!tabs)
			return;
		tabs.sort();
		// Покажем дополнительную панель
		if(current_tab == -1)
			current_tab = 0;
		auto h1 = 28;
		// Нарисуем закладки
		auto hilited = -1;
		if(draw::tabs({rc.x1, rc.y1, rc.x2, rc.y1 + h1 + 1}, false, false,
			(void**)tabs.data, 0, tabs.count, current_tab, &hilited,
			get_page_name)) {
			draw::execute(callback_settab);
			hot.param = hilited;
		}
		line(rc.x1, rc.y1 + h1, rc.x2, rc.y1 + h1, colors::border);
		// Нариуем текущую закладку
		if(current_tab == -1)
			return;
		auto p1 = tabs[current_tab];
		strowsa elements; elements.select_groups(p1);
		if(!elements)
			return;
		elements.sort();
		int w3, w4;
		auto e1 = p1->elements.begin();
		switch(e1->var.type) {
		case setting::Control:
			rc.y1 += h1;
			if(metrics::show::padding)
				rc.y1 += metrics::padding;
			((control*)e1->var.data)->show_border = metrics::show::padding;
			((control*)e1->var.data)->view(rc);
			break;
		default:
			rc.y1 += h1 + metrics::padding;
			w4 = rc.width();
			w3 = imin(w4, 620);
			for(auto pe : elements)
				rc.y1 += render_element(rc.x1, rc.y1, w3, 0, *pe);
			break;
		}
	}
	widget_settings() {
		show_background = false;
		show_border = false;
	}
} widget_settings_control;

static void postactivate() {
	activate((control*)hot.object);
}

static void postclose() {
	close((control*)hot.object);
}

static struct widget_application : draw::controls::control {
	fnevent			heartproc;
	control*		hotcontrols[48];
	const char* getlabel(stringbuilder& sb) const override {
		return "Главный";
	}
	bool isfocusable() const override {
		return false;
	}
	static aref<control*> getactivepages(control** result, unsigned count) {
		auto ps = result;
		auto pe = result + count;
		for(auto p : active_controls) {
			if(ps < pe)
				*ps++ = p;
		}
		return aref<control*>(result, ps - result);
	}
	static void show_statusbar(const control* pc) {
		char temp[260]; stringbuilder sb(temp);
		auto pu = pc->geturl(sb);
		if(pu)
			statusbar("Данные из %1", pu);
	}
	static void workspace(rect rc) {
		control* p1[64];
		auto c1 = getdocked(p1, sizeof(p1)/sizeof(p1[0]), DockWorkspace);
		auto c2 = getactivepages(c1.end(), (p1 + sizeof(p1) / sizeof(p1[0])) - c1.end());
		aref<control*> ct(c1.begin(), c1.getcount() + c2.getcount());
		if(!ct) {
			auto push_fore = fore;
			fore = colors::border;
			text(rc, "Не найдено ни одного открытого документа", AlignCenterCenter);
			fore = push_fore;
		} else if(ct.getcount() == 1) {
			current_active_control = p1[0];
			current_active_control->show_border = metrics::show::padding;
			current_active_control->view(rc);
		} else if(ct) {
			auto current_select = ct.indexof(current_active_control);
			if(current_select == -1)
				current_select = 0;
			const int dy = draw::texth() + 8;
			rect rct = {rc.x1, rc.y1, rc.x2, rc.y1 + dy};
			auto current_hilite = -1;
			auto result = draw::tabs(rct, false, false, (void**)c1.begin(), 0, c1.getcount(),
				current_select, &current_hilite, controls::getlabel, {2, 0, 2, 0}, &rct.x1);
			if(c2) {
				auto current_hilite2 = -1;
				auto r1 = draw::tabs(rct, true, false, (void**)ct.begin(), c1.getcount(), ct.getcount(),
					current_select, &current_hilite2, controls::getlabel, {2, 0, 2, 0}, &rct.x1);
				if(current_hilite2 != -1)
					current_hilite = current_hilite2;
				if(r1)
					result = r1;
			}
			if(current_hilite != -1) {
				show_statusbar(ct.begin()[current_hilite]);
				switch(result) {
				case 1: execute(postactivate, 0, 0, ct.begin()[current_hilite]); break;
				case 2: execute(postclose, 0, 0, ct.begin()[current_hilite]); break;
				}
			}
			rc.y1 += dy;
			if(current_active_control) {
				current_active_control->show_border = metrics::show::padding;
				current_active_control->view(rc);
			}
		}
	}
	void view(const rect& rc) override {
		auto rct = rc;
		for(auto p = controls::control::plugin::first; p; p = p->next) {
			auto pc = p->getcontrol();
			if(!pc)
				continue;
			pc->show_border = metrics::show::padding;
		}
		if(heartproc)
			heartproc();
		dockbar(rct);
		workspace(rct);
	}
	static control::command commands_general[];
	static control::command commands[];
	const command* getcommands() const override {
		return commands;
	}
	bool create_window(bool run) {
		return true;
	}
	void create_filter(stringbuilder& sb) {
		io::plugin::addfilter(sb, "Все файлы", "*.*");
		for(auto p = plugin::first; p; p = p->next) {
			auto pc = p->getbuilder();
			if(!pc)
				continue;
			pc->getextensions(sb);
		}
		sb.addsz();
	}
	bool open_window(bool run) {
		if(run) {
			char filter[1024 * 16]; stringbuilder sb(filter); create_filter(sb);
			if(!dialog::open("Открыть файл", last_open_file, filter, -1))
				return false;
			openurl(last_open_file);
		}
		return true;
	}
	bool save_window(bool run) {
		auto p = getactivated();
		if(!p)
			return false;
		if(!p->ismodified())
			return false;
		if(run) {

		}
		return false;
	}
	bool cut(bool run) {
		auto p = getfocus();
		if(!p)
			return false;
		return p->cut(run);
	}
	bool copy(bool run) {
		auto p = getfocus();
		if(!p)
			return false;
		return p->copy(run);
	}
	bool paste(bool run) {
		auto p = getfocus();
		if(!p)
			return false;
		return p->paste(run);
	}
	widget_application() {
		show_background = false;
		show_border = false;
		memset(hotcontrols, 0, sizeof(hotcontrols));
	}
} widget_application_control;

control::command widget_application::commands_general[] = {{"create", "Создать", 0, &widget_application::create_window, 0},
{"open", "Открыть", 0, &widget_application::open_window, 1},
{"save", "Сохранить", 0, &widget_application::save_window, 2},
{}};
control::command widget_application::commands[] = {{"*", "", commands_general},
{"*", "", commands_edit},
{}};

control* controls::getactivated() {
	return current_active_control;
}

void controls::activate(control* p) {
	if(current_active_control)
		current_active_control->deactivating();
	current_active_control = p;
	if(current_active_control) {
		current_active_control->setfocus(true);
		current_active_control->activating();
	}
}

void controls::close(control* p) {
	auto i = active_controls.indexof(p);
	if(i == -1)
		return;
	active_controls.remove(i, 1);
	if(current_active_control == p) {
		if(i > 0)
			activate(active_controls[i - 1]);
		else
			activate(0);
	}
	delete p;
}

control* controls::openurl(const char* url) {
	char temp[260];
	for(auto p : active_controls) {
		stringbuilder sb(temp);
		auto purl = p->geturl(sb);
		if(!purl)
			continue;
		if(szcmpi(purl, url) == 0) {
			activate(p);
			return p;
		}
	}
	for(auto p = control::plugin::first; p; p = p->next) {
		auto pc = p->getbuilder();
		if(!pc)
			continue;
		if(!pc->canopen(url))
			continue;
		if(pc->read(url))
			return 0;
		auto result = pc->create(url);
		if(result) {
			active_controls.add(result);
			activate(result);
			return result;
		}
	}
	return 0;
}

void set_light_theme();
void set_dark_theme();

static const element appearance_general_metrics[] = {{"Отступы", metrics::padding, 3},
{"Ширина скролла", metrics::scroll, 3},
};
static const element colors_general[] = {{"Установить светлую тему", set_light_theme},
{"Установить темную тему", set_dark_theme},
};
static const element colors_form[] = {{"Цвет текста", colors::text},
{"Цвет окна", colors::window},
{"Цвет формы", colors::form},
{"Цвет границы", colors::border},
{"Активный цвет", colors::active},
{"Цвет кнопки", colors::button},
{"Цвет редактирования", colors::edit},
{"Цвет закладок", colors::tabs::back},
{"Цвет текста закладок", colors::tabs::text},
{"Цвет подсказки", colors::tips::back},
{"Цвет текста подсказки", colors::tips::text},
};
static const element appearance_general_view[] = {{"Показывать панель статуса", metrics::show::statusbar},
{"Показывать левую панель элементов", metrics::show::left},
{"Показывать правую панель элементов", metrics::show::right},
{"Показывать нижнюю панель элементов", metrics::show::bottom},
{"Отступы на главном окне", metrics::show::padding},
{"Использовать оптимизацию при движении мишки", sys_optimize_mouse_move},
};
static const element appearance_general_tabs[] = {{"В имени закладки отображать только имя файла (без полного пути)", use_short_name_label},
{"Не выводить в имя закладки расширение", use_no_extension_label},
{"Первую букву имени закладки выводить в верхнем регистре", use_uppercase_label},
};
static const element plugin_elements[] = {{0, {Control, static_cast<control*>(&control_viewer), 0}}};
static header setting_headers[] = {{"Рабочий стол", "Общие", "Метрика", appearance_general_metrics},
{"Рабочий стол", "Общие", "Внешний вид", appearance_general_view},
{"Рабочий стол", "Общие", "Закладки", appearance_general_tabs},
{"Цвета", "Общие", 0, colors_general},
{"Цвета", "Формы", 0, colors_form},
{"Рабочий стол", "Окна", 0, plugin_elements},
};

static struct application_plugin : draw::initplugin {
	void initialize() override {
		control_viewer.initialize();
	}
	static void exit_application() {
		point pos, size; getwindowpos(pos, size, &window.flags);
		if((window.flags&WFMaximized) == 0) {
			window.x = pos.x;
			window.y = pos.y;
			window.width = size.x;
			window.height = size.y;
		}
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
	char temp[260]; stringbuilder sb(temp);
	draw::statusbar("Переключить вид на '%1'", object->getlabel(sb));
}

bool draw::controls::edit(control& e, fnevent heartbeat) {
	while(ismodal()) {
		auto tb = e.getimages();
		rect rc = {0, 0, draw::getwidth(), draw::getheight()};
		draw::rectf(rc, colors::form);
		if(metrics::show::statusbar)
			rc.y2 -= draw::statusbardw();
		rect rt = rc;
		if(tb)
			rt.y2 = rt.y1 + tb->get(0).getrect(0, 0, 0).height() + 4 * 2;
		else
			rt.y2 = rt.y1 + 24 + 4 * 2;
		sheetline(rt, true);
		rc.y1 += rt.height();
		rt.x1 += 2; rt.y1 += 1; rt.x2 -= 2;
		auto dx = draw::texth() + metrics::padding + 4*2;
		auto y2 = rc.y2 - dx; rc.y2 = y2;
		if(metrics::show::padding) {
			rc.x1 += metrics::padding;
			rc.x2 -= metrics::padding;
			rc.y1 += metrics::padding;
		}
		auto x2 = rc.x2;
		e.toolbar(rt.x1, rt.y1, rt.width());
		e.view(rc);
		draw::buttonr(x2, y2, buttoncancel, "Cancel", &e, KeyEscape);
		draw::buttonr(x2, y2, buttonok, "OK", &e, Ctrl + KeyEnter);
		domodal();
		if(heartbeat)
			heartbeat();
	}
	return getresult() != 0;
}

void draw::application(fnevent heartproc, shortcut* shortcuts) {
	// Make header
	setting_header.initialize();
	widget_application_control.heartproc = heartproc;
	auto current_tab = 0;
	while(ismodal()) {
		auto pc = layouts[current_tab];
		auto tb = pc->getimages();
		draw::fore = colors::text;
		rect rc = {0, 0, draw::getwidth(), draw::getheight()};
		draw::rectf(rc, colors::form);
		if(metrics::show::statusbar)
			rc.y2 -= draw::statusbardw();
		rect rt = rc;
		if(tb)
			rt.y2 = rt.y1 + tb->get(0).getrect(0, 0, 0).height() + 4 * 2;
		else
			rt.y2 = rt.y1 + 24 + 4 * 2;
		sheetline(rt, true);
		rc.y1 += rt.height();
		rt.x1 += 2; rt.y1 += 1; rt.x2 -= 2;
		if(metrics::show::padding)
			rc.offset(metrics::padding);
		pc->view(rc);
		pc->toolbar(rt.x1, rt.y1, rt.width());
		auto hilite_tab = -1;
		auto reaction = draw::tabs(rt, false, true, (void**)layouts, 0,
			sizeof(layouts) / sizeof(layouts[0]), current_tab, &hilite_tab,
			controls::getlabel,
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
		default:
			if(shortcuts)
				execute(shortcuts);
			break;
		}
	}
}

void set_dark_theme();
void set_light_theme();

void draw::application_initialize() {
	set_light_theme();
	initialize();
	create(window.x, window.y, window.width, window.height, window.flags, 32);
}

void draw::application(const char* name, fnevent showproc, fnevent heartproc, shortcut* shortcuts) {
	application_initialize();
	setcaption(name);
	if(showproc)
		showproc();
	application(heartproc, shortcuts);
}

static int getnum(const char* value) {
	if(strcmp(value, "true") == 0)
		return 1;
	if(strcmp(value, "false") == 0)
		return 0;
	if(strcmp(value, "null") == 0)
		return 0;
	return sz2num(value);
}

static struct settings_settings_strategy : io::strategy {
	void write(serializer& file, const element& e) {
		switch(e.var.type) {
		case setting::Bool:
		case setting::Number:
		case setting::Color:
			file.set(e.name, e.var.get());
			break;
		case setting::Radio:
			file.set(e.name, 1);
			break;
		case setting::Text:
		case setting::Url:
			file.set(e.name, (const char*)e.var.get());
			break;
		default:
			break;
		}
	}
	void write(serializer& file, void* param) override {
		strowsa divisions; divisions.select_divisions();
		for(auto pd : divisions) {
			strowsa pages; pages.select_pages(pd);
			if(!pages)
				continue;
			file.open(pd->division);
			for(auto pg : pages) {
				strowsa groups; groups.select_groups(pg);
				if(!groups)
					continue;
				file.open(pg->page);
				for(auto gr : groups) {
					if(gr->group)
						file.open(gr->group);
					for(auto& e : gr->elements)
						write(file, e);
					if(gr->group)
						file.close(gr->group);
				}
				file.close(pg->page);
			}
			file.close(pd->division);
		}
	}
	static const element* find(serializer::node& n) {
		auto name = n.name;
		if(!name)
			return 0;
		if(!n.parent || !n.parent->parent || !n.parent->parent->parent)
			return 0;
		const char* division = 0;
		const char* page = 0;
		const char* group = 0;
		if(strcmp(n.parent->parent->parent->name, "settings") == 0) {
			division = n.parent->parent->name;
			page = n.parent->name;
		} else {
			if(!n.parent->parent->parent->parent)
				return 0;
			if(strcmp(n.parent->parent->parent->parent->name, "settings") != 0)
				return 0;
			division = n.parent->parent->parent->name;
			page = n.parent->parent->name;
			group = n.parent->name;
		}
		if(!division || !page)
			return 0;
		for(auto pm = header::first; pm; pm = pm->next) {
			if(strcmp(pm->division, division) != 0)
				continue;
			if(strcmp(pm->page, page) != 0)
				continue;
			if(pm->group) {
				if(!group || strcmp(pm->group, group) != 0)
					continue;
			} else {
				if(group)
					continue;
			}
			for(auto& e : pm->elements) {
				if(strcmp(e.name, name) == 0)
					return &e;
			}
		}
		return 0;
	}
	void set(serializer::node& n, const char* value) override {
		auto e = find(n);
		if(!e)
			return;
		switch(e->var.type) {
		case setting::Number:
		case setting::Color:
		case setting::Bool:
			e->var.set(getnum(value));
			break;
		case setting::Radio:
			e->var.set(e->var.value);
			break;
		case setting::Text:
		case setting::Url:
			e->var.set((int)szdup(value));
			break;
		default:
			break;
		}
	}
	settings_settings_strategy() : strategy("settings", "settings") {}
} settings_settings_strategy_instance;

static struct window_settings_strategy : io::strategy {
	void write(serializer& file, void* param) override {
		file.set("x", window.x);
		file.set("y", window.y);
		file.set("width", window.width);
		file.set("height", window.height);
		file.set("header_width", window.header_width);
		file.set("flags", window.flags);
	}
	void set(serializer::node& n, const char* value) override {
		if(n == "x")
			window.x = getnum(value);
		else if(n == "y")
			window.y = getnum(value);
		else if(n == "width")
			window.width = getnum(value);
		else if(n == "height")
			window.height = getnum(value);
		else if(n == "header_width")
			window.header_width = getnum(value);
		else if(n == "flags")
			window.flags = getnum(value);
	}
	window_settings_strategy() : strategy("window", "settings") {}
} window_settings_strategy_instance;

static struct controls_settings_strategy : io::strategy {
	void write(serializer& file, void* param) override {
		for(auto pp = controls::control::plugin::first; pp; pp = pp->next) {
			auto id = pp->id;
			if(!id || id[0] == 0)
				continue;
			file.open(id);
			file.set("Docking", pp->dock);
			file.set("Visible", pp->visible);
			auto pc = pp->getcontrol();
			if(!pc)
				continue;
			pc->write(file);
			file.close(id);
		}
	}
	void set(serializer::node& n, const char* value) override {
		if(!n.parent || !n.parent->parent)
			return;
		auto e = const_cast<controls::control::plugin*>(controls::control::plugin::find(n.parent->name));
		if(!e)
			return;
		else if(n == "Docking")
			e->dock = (dock_s)sz2num(value);
		else if(n == "Visible")
			e->visible = getnum(value);
	}
	controls_settings_strategy() : strategy("controls", "settings") {}
} controls_settings_strategy_instance;