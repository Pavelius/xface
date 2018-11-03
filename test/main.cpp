#include "xface/bsdata.h"
#include "xface/bsreq.h"
#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/draw_control.h"
#include "xface/draw_grid.h"
#include "xface/variable.h"
#include "xface/requisit.h"

using namespace draw;

static unsigned radio_button = 2;
static unsigned check_button = 0;

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

struct testinfo {
	const char*		name;
	int				value;
};

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

static void button_accept() {
	basic_drawing();
}

static void test_control() {
	struct element {
		const char*		name;
		gender_s		gender;
		alignment_s		alignment;
	};
	static controls::column columns[] = {{Field, "name", "Наименование", 200},
	{Check, "check", 0, 28, "Это тултипс колонки"},
	{Field, "gender", "Пол", 128},
	{Field, "count", "К-во", 48},
	{}};
	static bsreq element_type[] = {
		BSREQ(element, name, text_type),
		BSREQ(element, gender, gender_type),
		BSREQ(element, alignment, alignment_type),
	{}};
	adat<element, 32> elements;
	elements.add({"Pavel", Male, ChaoticEvil});
	elements.add({"Olga", Female, ChaoticGood});
	elements.add({"Valentin", Male, NeutralGood});
	controls::grid test(columns, element_type, elements);//sizeof(element));
	test.no_change_order = false;
	while(ismodal()) {
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		rc.offset(4 * 2);
		rc.y2 -= button(rc.x2 - 100 + metrics::padding, rc.y2 - draw::texth() - metrics::padding * 3, 100, 0, cmd(buttoncancel), "Назад");
		rc.y1 += test.toolbar(rc.x1, rc.y1, rc.width());
		test.view(rc);
		domodal();
	}
}

static void test_widget() {
	struct element {
		const char*		surname;
		const char*		name;
		int				mark;
		char			radio;
		char			age;
		gender_s		gender;
		alignment_s		alignment;
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
	static widget elements_left[] = {{Radio, "radio", "Samsung", 0},
	{Radio, "radio", "Nokia", 1},
	{Radio, "radio", "Google", 2},
	{}};
	static widget elements_right[] = {{Radio, "radio", "Noobie", 3},
	{Radio, "radio", "Glass", 4},
	{Radio, "radio", "Keeps", 5},
	{}};
	static widget brands[] = {//{Image, "cat", "art/pictures", 0, 5},
		{Group, 0, 0, 0, 3, 0, 0, elements_left},
	{Group, 0, 0, 0, 3, 0, 0, elements_right},
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
		draw::render(10, 10, 500, bsval(element_type, &test), elements);
		domodal();
	}
}

static const char* get_text(char* result, const char* result_maximum, void* object) {
	return (const char*)object;
}

static void disabled_button() {
}

static void simple_controls() {
	static const char* elements[] = {"Файл", "Правка", "Вид", "Окна"};
	setfocus(3, true);
	int current_hilite;
	const char* t1 = "Тест 1";
	char t2 = 20;
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		statusbardw();
		auto x = 10; auto y = 10;
		auto result = tabs(x, y, getwidth() - x * 2, false, false, (void**)elements, 0, sizeof(elements) / sizeof(elements[0]), 0, &current_hilite,
			get_text, {}); y += 40;
		if(current_hilite != -1)
			statusbar("Выбрать закладку '%1'", elements[current_hilite]);
		y += button(x, y, 200, button_accept, "Графические примитивы", "Кнопка, которая отображает подсказку");
		y += button(x, y, 200, test_control, "Элемент управления");
		y += button(x, y, 200, test_widget, "Виджеты");
		y += button(x, y, 200, Disabled, cmd(disabled_button), "Недоступная кнопка", "Кнопка, которая недоступная для нажатия");
		domodal();
	}
}

static void test_array() {
	avec a1(sizeof(testinfo));
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

using namespace compiler;

static void test_requisit() {
	auto p2 = compiler::object->add("pointer");
	p2->add("x", number);
	p2->add("y", number);
	auto p1 = compiler::object->add("rect");
	p1->add("x1", number);
	p1->add("y1", number);
	p1->add("x2", number);
	p1->add("y2", number);
}

int main() {
	test_array();
	set_light_theme();
	// Инициализация библиотеки
	initialize();
	// Создание окна
	create(-1, -1, 640, 480, WFResize | WFMinmax, 32);
	setcaption("X-Face C++ library");
	simple_controls();
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}