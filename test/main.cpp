#include "xface/bsdata.h"
#include "xface/bsreq.h"
#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/datetime.h"
#include "xface/draw_control.h"
#include "xface/draw_grid.h"
#include "xface/requisit.h"
#include "xface/stringcreator.h"

using namespace draw;

void logmsg(const char* format, ...);

static unsigned radio_button = 2;
static unsigned check_button = 0;

extern bsdata cultivated_land_manager;

static const char* product_category[] = {"Shoe", "T-Short", "Cap", "Book", "Phone", "Smartphone", "Pencil",
"Keyboard", "Mouse", "Headphones",
"Car", "Bus", "Flashmemory",
"Mute", "Guitar", "Piano",
"Soldier", "Heavy Soldier", "Sniper", "Commando"
};

enum alignment_s : unsigned char {
	LawfulGood, NeutralGood, ChaoticGood,
	LawfulNeutral, TrueNeutral, ChaoticNeutral,
	LawfulEvil, NeutralEvil, ChaoticEvil,
};
enum gender_s : unsigned char {
	NoGender, Male, Female,
};

static struct gender_info {
	const char*		name;
} gender_data[] = {{"Неизвестен"},
{"Мужчина"},
{"Женщина"},
};
static bsreq gender_type[] = {
	BSREQ(gender_info, name, text_type),
{}};
BSMETA(gender);

static struct alignment_info {
	const char*		id;
	const char*		name;
} alignment_data[] = {{"Neutral", "Нейтральный"},
{"Lawful Good", "Законопослушный добрый"},
{"Neutral Good", "Нейтрально добрый"},
{"Chaotic Good", "Хаотично добрый"},
{"Lawful Neutral", "Законопослушный нейтральный"},
{"Chaotic Neutral", "Хаотично нейтральный"},
{"Lawful Evil", "Законопослушный злой"},
{"Neutral Evil", "Нейтрально злой"},
{"Chaotic Evil", "Хаотично злой"},
};
assert_enum(alignment, ChaoticEvil);
getstr_enum(alignment);
static bsreq alignment_type[] = {
	BSREQ(alignment_info, id, text_type),
	BSREQ(alignment_info, name, text_type),
{}};
BSMETA(alignment);

static struct element {
	const char*		surname;
	const char*		name;
	int				mark;
	char			radio;
	char			age;
	gender_s		gender;
	alignment_s		alignment;
} element_data[] = {{"Ivanov", "Ivan", 0, 0, 20, Male, ChaoticGood},
{"Petrov", "Peter", 0, 0, 25, Male, ChaoticNeutral},
{"Ludina", "Iren", 0, 0, 25, Female, LawfulGood},
};
static bsreq element_type[] = {
	BSREQ(element, mark, number_type),
	BSREQ(element, radio, number_type),
	BSREQ(element, name, text_type),
	BSREQ(element, surname, text_type),
	BSREQ(element, age, number_type),
	BSREQ(element, gender, gender_type),
	BSREQ(element, alignment, alignment_type),
{}};

struct testinfo {
	const char*		name;
	int				value;
};

struct cmd_value : runable {

	callback	id;
	void*			param;

	constexpr cmd_value(callback id, void* param) : id(id), param(param) {}
	constexpr cmd_value(bool& value) : cmd_value(execute_bool, &value) {}

	void execute() const override {
		draw::execute(id, (int)param);
	}

	int	getid() const override {
		return (int)param;
	}


	static void execute_bool() {
		auto v = *((bool*)hot.param);
		*((bool*)hot.param) = !v;
	}

};

static int checkbox(int x, int y, int width, bool& value, const char* title, const char* tips = 0) {
	return checkbox(x, y, width,
		value ? Checked : 0, cmd_value(value), title)
		+ metrics::padding;
}

static int button(int x, int y, int width, void(*proc)(), const char* title, const char* tips = 0) {
	auto result = button(x, y, width, 0, cmd(proc), title, tips);
	rect rc = {x, y, x + width, y + texth() + metrics::padding * 2};
	if(areb(rc)) {
		if(tips)
			statusbar("Кнопка: %1", tips);
		else
			statusbar("Кнопка с координатами x=%1i, y=%2i", x, y);
	}
	return result;
}

static int show_control(controls::control& e) {
	while(ismodal()) {
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		rc.offset(4 * 2);
		rc.y2 -= button(rc.x2 - 100 + metrics::padding, rc.y2 - draw::texth() - metrics::padding * 3, 100, 0, cmd(buttoncancel), "Назад", 0, KeyEscape);
		rc.y1 += e.toolbar(rc.x1, rc.y1, rc.width());
		e.view(rc);
		domodal();
	}
	return getresult();
}

static int show_table(controls::table& e) {
	setfocus((int)&e, true);
	while(ismodal()) {
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		rc.offset(4 * 2);
		button(rc.x2 - 100 + metrics::padding, rc.y2 - draw::texth() - metrics::padding * 2, 100, 0, cmd(buttoncancel), "Назад", 0, KeyEscape);
		auto y0 = rc.y2 - (draw::texth() + metrics::padding) * 3;
		auto x1 = rc.x1, y1 = y0, width = 300;
		y1 += checkbox(x1, y1, width, e.show_grid_lines, "Показывть линии");
		y1 += checkbox(x1, y1, width, e.show_header, "Показывть заголовок");
		y1 += checkbox(x1, y1, width, e.hilite_odd_lines, "Подсвечивать нечетные рядки");
		x1 += width + metrics::padding, y1 = y0;
		y1 += checkbox(x1, y1, width, e.show_totals, "Показывть итоги");
		y1 += checkbox(x1, y1, width, e.show_border, "Показывть границы");
		y1 += checkbox(x1, y1, width, e.show_background, "Показывать фон");
		rc.y2 = y0 - metrics::padding;
		rc.y1 += e.toolbar(rc.x1, rc.y1, rc.width());
		e.view(rc);
		domodal();
	}
	return getresult();
}

static void basic_drawing() {
	static int tick = 10;
	settimer(100);
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		auto x = 100 + tick % 100;
		auto y = 100 + tick % 200;
		circlef(x, y, 50 + tick % 40, colors::form);
		circle(x, y, 50 + tick % 40, colors::border);
		button(10, 10, 100, 0, cmd(buttoncancel), "Отмена", 0, KeyEscape);
		tick++;
		domodal();
	}
	settimer(0);
}

static void many_lines() {
	draw::state push;
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		auto x = 100;
		auto y = 100;
		linw = 1;
		line(x, y, x + 100, y + 100); y += 40; linw = 2;
		line(x, y, x + 100, y + 100); y += 40; linw = 3;
		line(x, y, x + 100, y + 100); y += 40; linw = 4;
		line(x, y, x + 100, y + 100); y += 40; linw = 5;
		line(x, y, x + 60, y + 100);
		rectb({300, 100, 500, 300}, colors::green);
		linw = 1.0;
		button(10, 10, 100, 0, cmd(buttoncancel), "Отмена", 0, KeyEscape);
		domodal();
	}
}

static void test_grid() {
	struct element {
		const char*		name;
		gender_s		gender;
		alignment_s		alignment;
		char			image;
		datetime		date;
	};
	static bsreq element_type[] = {
		BSREQ(element, name, text_type),
		BSREQ(element, gender, gender_type),
		BSREQ(element, alignment, alignment_type),
		BSREQ(element, date, number_type),
		BSREQ(element, image, number_type),
	{}};
	adat<element, 32> elements;
	elements.add({"Pavel", Male, ChaoticEvil, 2, datetime::now()});
	elements.add({"Olga", Female, ChaoticGood, 0, datetime::now() - 5*24*60});
	elements.add({"Valentin", Male, NeutralGood, 1, datetime::now() - 3 * 24 * 60});
	elements.add({"Jorgun", Male, LawfulGood, 0, datetime::now() - 4 * 24 * 60});
	controls::grid test(element_type, elements);
	test.addcol("image", 0, "image");
	test.addcol("name", "Наименование", "text");
	test.addcol("gender", "Пол", "ref");
	test.addcol("alignment", "Мировозрение", "ref");
	test.addcol("alignment.id", "Мировозрение (англ)", "text");
	test.addcol("date", "Дата", "datetime");
	test.no_change_order = false;
	test.show_grid_lines = true;
	test.read_only = false;
	test.no_change_count = false;
	show_table(test);
}

static void test_grid_ref() {
	controls::gridref test(cultivated_land_manager.fields);
	test.addcol("name", "Наименование", "text", SizeAuto);
	test.addcol("cult_land", "Обрабатывается", "number");
	test.addcol("cult_land_percent", "Обрабатывается (%)", "percent");
	test.add(cultivated_land_manager.get(0));
	test.add(cultivated_land_manager.get(1));
	test.add(cultivated_land_manager.get(1));
	test.no_change_order = false;
	test.show_grid_lines = true;
	test.read_only = false;
	test.no_change_count = false;
	show_table(test);
}

static void test_tree() {
	struct test_tree_control : controls::tree {
		void expanding(builder&  e) {
			e.add(cultivated_land_manager.get(0), 0, 0, false);
			e.add(cultivated_land_manager.get(1), 1, 0, true);
			e.add(cultivated_land_manager.get(2), 1, 0, true);
		}
		constexpr test_tree_control() : tree(cultivated_land_manager.fields){}
	} test;
	test.select_mode = SelectText;
	test.addcol("image", 0, "image", SizeInner);
	test.addcol("name", "Наименование", "text", SizeAuto);
	test.addcol("cult_land", "Обрабатывается", "number");
	test.addcol("cult_land_percent", "Обрабатывается (%)", "percent");
	test.expand(0, 0);
	show_table(test);
}

static void test_widget() {
	static widget elements_left[] = {{Radio, "radio", "Samsung", 0},
	{Radio, "radio", "Nokia", 1},
	{Radio, "radio", "Google", 2},
	{}};
	static widget elements_right[] = {{Radio, "radio", "Noobie", 3},
	{Radio, "radio", "Glass", 4},
	{Radio, "radio", "Keeps", 5},
	{}};
	static widget brands[] = {{Group, 0, 0, 0, 3, 0, 0, elements_left},
	{Group, 0, 0, 0, 3, 0, 0, elements_right},
	{Image, "cat", "art/pictures", 0, 5},
	{}};
	static widget field_group_left[] = {{Field, "name", "Имя"},
	{Field, "surname", "Фамилия"},
	{Field, "age", "Возраст"},
	{Field, "gender", "Пол", 0, 0, 0, 0, 0, "Этот элемент позволяет произвести выбор из списка"},
	{Field, "alignment", "Мировозрение"},
	{}};
	static widget field_group_right[] = {{Button, "button1", "Отмена", 0, 0, 0, 0, 0, 0, KeyEscape, buttoncancel},
	{}};
	static widget field_group[] = {{Group, 0, 0, 0, 8, 0, 0, field_group_left},
	{Group, 0, 0, 0, 4, 0, 0, field_group_right},
	{}};
	static widget elements[] = {{Label, 0, "A **character** who uses a weapon without being proficient with it suffers a [--4] penalty on attack rolls. The character can gain this feat multiple times.Each time the character takes the feat, it applies to a new weapon. A cleric whose deity's favored weapon is a martial weapon and who chooses War as one of his domains receives the Martial Weapon Proficiency feat related to that weapon for free, as well as the [Weapon Focus] feat related to that weapon."},
	{Check, "mark", "Простая пометка"},
	{Group, 0, "Выбирайте брэнд", 0, 0, 0, 0, brands},
	{Group, 0, 0, 0, 0, 0, 0, field_group},
	{}};
	element test = {0};
	test.gender = Male;
	test.mark = 1;
	test.radio = 2;
	while(ismodal()) {
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		draw::render(10, 10, 500, bsval(&test, element_type), elements);
		domodal();
	}
}

static void test_list() {
	struct test_class : controls::list {
		const char* getname(char* result, const char* result_maximum, int line, int column) const override {
			return product_category[line];
		}
		int getmaximum() const override {
			return sizeof(product_category) / sizeof(product_category[0]);
		}
	} test;
	show_control(test);
}

static const char* get_text(char* result, const char* result_maximum, void* object) {
	return (const char*)object;
}

static void disabled_button() {}

static void test_application() {
}

static void test_drag_drop() {
	auto x = 100, y = 200;
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		rect rc = {x, y, x + 50, y + 25};
		if(dragactive(test_drag_drop)) {
			auto sx = rc.width();
			auto sy = rc.height();
			rc.x1 = x + (hot.mouse.x - dragmouse.x);
			rc.y1 = y + (hot.mouse.y - dragmouse.y);
			rc.x2 = rc.x1 + sx;
			rc.y2 = rc.y1 + sy;
			char temp[260];
			text(10, 42, szprint(temp, zendof(temp), "Начинаем тягать %1i, %2i", rc.x1, rc.y1));
		} else if(areb(rc) && hot.key == MouseLeft && hot.pressed)
			dragbegin(test_drag_drop);
		if(dragactive(test_drag_drop)) {
			line(rc.x1, rc.y1, dragmouse.x, dragmouse.y, colors::red);
			line(rc.x2, rc.y1, dragmouse.x, dragmouse.y, colors::blue);
			line(rc.x1, rc.y2, dragmouse.x, dragmouse.y, colors::green);
			line(rc.x2, rc.y2, dragmouse.x, dragmouse.y, colors::yellow);
		}
		rectf(rc, colors::form);
		rectb(rc, colors::border);
		button(10, 10, 100, 0, cmd(buttoncancel), "Отмена", 0, KeyEscape);
		domodal();
	}
}

static void choose_folder(const storage& ev) {
	char temp[260] = {};
	if(!dialog::folder("Выбирайте папку", temp))
		return;
	ev.set(temp);
}

static int point_input(int x, int y, point& result, int width, int title, const char* t1, const char* t2) {
	auto y0 = y;
	y += field(x, y, width, 0, result.x, t1, 0, title);
	y += field(x, y, width, 0, result.y, t2, 0, title);
	return y - y0;
}

static void choose_transparent_color() {

}

static void test_tile_manager() {
	setfocus(0, true);
	char filename[260] = {};
	point tile = {};
	point origin = {};
	color transparent = {160, 160, 0};
	auto use_transparent = false;
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		auto x = 20, y = 20;
		auto h = draw::texth();
		y += field(x, y, 380, 0, filename, "Файл тайлов", 0, 100, choose_folder);
		y += checkbox(x, y, 380, use_transparent, "Использовать прозрачный цвет");
		if(use_transparent) {
			y += field(x, y, 380, 0, transparent, "Цвет", 0, 100);
			y += button(x + 100, y, 280, choose_transparent_color, "Выбрать цвет с картнки");
		}
		auto y0 = y;
		y += point_input(x, y, tile, 180, 100, "Ширина (точек)", "Высота (точек)");
		point_input(x + 200, y0, origin, 180, 100, "Смещение", "Отступ");
		y += button(x, y, 300, buttonok, "Принять");
		domodal();
	}
}

static void test_edit_field() {
	setfocus(0, true);
	char name[260] = "Павел";
	char surname[260] = "Чистяков";
	char lastname[260] = "Валенинович";
	const char* anystr = "Любая строка";
	int number = 10;
	while(ismodal()) {
		auto x = 20, y = 20;
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		auto h = draw::texth();
		auto w = 300;
		y += field(x, y, 300, 0, name, "Тест", "Теперь подсказки можно выводить прямо в поле ввода", 100);
		y += field(x, y, 300, 0, surname, "Еще тест", 0, 100);
		y += field(x, y, 300, 0, lastname, "Еще поле", 0, 100);
		y += field(x, y, 300, 0, anystr, "Путь к папке", 0, 100, choose_folder);
		y += field(x, y, 300, 0, number, "Скорость", 0, 100);
		y += button(x, y, 300, buttonok, "Принять", "Такая подсказка должна появляться всегда");
		domodal();
	}
}

static void start_menu() {
	struct element {
		const char*		name;
		callback	proc;
		const char*		tips;
	};
	static element element_data[] = {{"Графические примитивы", basic_drawing},
	{"Линии", many_lines},
	{"Перетаскивание", test_drag_drop},
	{"Список", test_list},
	{"Таблица с ячейками", test_grid},
	{"Таблица ссылок", test_grid_ref},
	{"Дерево", test_tree},
	{"Виджеты", test_widget},
	{"Поле ввода", test_edit_field},
	{"Тайлы", test_tile_manager},
	{"Приложение", draw::application},
	{0}};
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		statusbardw();
		auto x = 10; auto y = 10;
		for(auto p = element_data; p->name; p++)
			y += button(x, y, 200, p->proc, p->name, p->tips);
		domodal();
	}
}

static void test_array() {
	array a1(sizeof(testinfo));
	auto p1 = (testinfo*)a1.add();
	p1->name = "Test";
	p1->value = 1;
	auto test_count = a1.getcount();
	p1 = (testinfo*)a1.add();
	p1->name = "New test";
	p1->value = 2;
	testinfo b1 = {"Third test", 3};
	p1 = (testinfo*)a1.insert(0, &b1);
	p1->name = "Third test";
	p1->value = 2;
}

static void test_requisit() {
	using namespace compiler;
	manager man;
	auto s1 = man.get("Test");
	auto s2 = man.get("Test");
	auto s3 = man.get("Test2");
	auto s4 = man.get("All correct");
	auto p2 = man.create("pointer");
	man.add(p2, "x", Number, 1, 2);
	man.add(p2, "y", Number, 1, 2);
	auto p1 = man.create("rect");
	man.add(p1, "x1", Number, 1, 4);
	man.add(p1, "y1", Number, 1, 4);
	man.add(p1, "x2", Number, 1, 4);
	man.add(p1, "y2", Number, 1, 4);
	auto p3 = man.create("region");
	man.add(p2, "rect", man.reference(p1));
	man.add(p2, "point", man.reference(p2), 2);
}

static void test_binary_serial() {
	bsdata::write("test.mtd", element_data, element_type);
}

static bool test_map() {
	amap<const char*, int> map;
	map.add("Test", 1);
	map.add("Test", 2);
	map.add("Color", 3);
	map.add("Four", 4);
	auto v4 = map.get("Four");
	auto k4 = map.getv(4);
	if(v4 != 4)
		return false;
	return true;
}

static bool test_datetime() {
	datetime d = datetime::now().daybegin() + 24*60;
	auto iy = d.year();
	auto im = d.month();
	auto id = d.day();
	return true;
}

int main() {
	logmsg("Test %1i", 12);
	test_datetime();
	test_requisit();
	test_array();
	test_map();
	test_binary_serial();
	application_initialize();
	// Создание окна
	setcaption("X-Face C++ library samples");
	start_menu();
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}