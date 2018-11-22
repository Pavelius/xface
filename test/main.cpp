#include "xface/bsdata.h"
#include "xface/bsreq.h"
#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/draw_control.h"
#include "xface/draw_grid.h"
#include "xface/requisit.h"
#include "xface/stringcreator.h"

using namespace draw;

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

	callback_proc	id;
	void*			param;

	constexpr cmd_value(callback_proc id, void* param) : id(id), param(param) {}
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
		rc.y2 -= button(rc.x2 - 100 + metrics::padding, rc.y2 - draw::texth() - metrics::padding * 3, 100, 0, cmd(buttoncancel), "Назад");
		rc.y1 += e.toolbar(rc.x1, rc.y1, rc.width());
		e.view(rc);
		domodal();
	}
	return getresult();
}

static int show_table(controls::table& e) {
	while(ismodal()) {
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		rc.offset(4 * 2);
		rc.y2 -= button(rc.x2 - 100 + metrics::padding, rc.y2 - draw::texth() - metrics::padding * 3, 100, 0, cmd(buttoncancel), "Назад");
		rc.y2 -= checkbox(rc.x1, rc.y2 - draw::texth() - metrics::padding, rc.width(), e.show_grid_lines, "Показывть линии");
		rc.y2 -= checkbox(rc.x1, rc.y2 - draw::texth() - metrics::padding, rc.width(), e.show_header, "Показывть заголовок");
		rc.y2 -= checkbox(rc.x1, rc.y2 - draw::texth() - metrics::padding, rc.width(), e.hilite_odd_lines, "Подсвечивать нечетные рядки");
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
		button(getwidth() - 110, 10, 100, buttoncancel, "Отмена");
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
		button(getwidth() - 110, 10, 100, buttoncancel, "Отмена");
		domodal();
	}
}

static void test_grid() {
	struct element {
		const char*		name;
		gender_s		gender;
		alignment_s		alignment;
		char			image;
	};
	static bsreq element_type[] = {
		BSREQ(element, name, text_type),
		BSREQ(element, gender, gender_type),
		BSREQ(element, alignment, alignment_type),
	{}};
	adat<element, 32> elements;
	elements.add({"Pavel", Male, ChaoticEvil});
	elements.add({"Olga", Female, ChaoticGood});
	elements.add({"Valentin", Male, NeutralGood});
	elements.add({"Jorgun", Male, LawfulGood, 1});
	controls::grid test(element_type, elements);
	test.addcol("image", 0, "image");
	test.addcol("name", "Наименование", "text");
	test.addcol("gender", "Пол", "ref");
	test.addcol("alignment", "Мировозрение", "ref");
	test.addcol("alignment.id", "Мировозрение (англ)", "text");
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
			e.add(cultivated_land_manager.get(1), 1, 0, true);
			e.add(cultivated_land_manager.get(2), 1, 0, true);
			e.add(cultivated_land_manager.get(0), 0, 0, false);
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

static void test_drag_drop() {
	auto x = 100, y = 200;
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		rect rc = {x, y, x + 50, y + 25};
		if(drag::active(1)) {
			auto sx = rc.width();
			auto sy = rc.height();
			rc.x1 = x + (hot.mouse.x - drag::mouse.x);
			rc.y1 = y + (hot.mouse.y - drag::mouse.y);
			rc.x2 = rc.x1 + sx;
			rc.y2 = rc.y1 + sy;
			char temp[260];
			text(10, 10, szprints(temp, zendof(temp), "Начинаем тягать %1i, %2i", rc.x1, rc.y1));
		} else if(areb(rc) && hot.key == MouseLeft && hot.pressed)
			drag::begin(1);
		rectf(rc, colors::form);
		rectb(rc, colors::border);
		button(getwidth() - 110, 10, 100, buttoncancel, "Отмена");
		domodal();
	}
}

static void start_menu() {
	struct element {
		const char*		name;
		callback_proc	proc;
		const char*		tips;
	};
	static element element_data[] = {{"Графические примитивы", basic_drawing},
	{"Линии", many_lines},
	{"Список", test_list},
	{"Таблица с ячейками", test_grid},
	{"Таблица ссылок", test_grid_ref},
	{"Дерево", test_tree},
	{"Виджеты", test_widget},
	{"Перетаскивание", test_drag_drop},
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

void set_dark_theme();
void set_light_theme();

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

int main() {
	test_requisit();
	test_array();
	test_map();
	test_binary_serial();
	set_light_theme();
	// Инициализация библиотеки
	initialize();
	// Создание окна
	create(-1, -1, 640, 480, WFResize | WFMinmax, 32);
	setcaption("X-Face C++ library");
	start_menu();
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}