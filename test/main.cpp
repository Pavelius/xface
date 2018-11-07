#include "xface/bsdata.h"
#include "xface/bsreq.h"
#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/draw_control.h"
#include "xface/draw_grid.h"
#include "xface/variable.h"
#include "xface/requisit.h"
#include "xface/stringcreator.h"

using namespace draw;

static unsigned radio_button = 2;
static unsigned check_button = 0;

extern bsdata cultivated_land_manager;

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
} gender_data[] = {{"����������"},
{"�������"},
{"�������"},
};
static bsreq gender_type[] = {
	BSREQ(gender_info, name, text_type),
{}};
BSMETA(gender);

static struct alignment_info {
	const char*		id;
	const char*		name;
} alignment_data[] = {{"Neutral", "�����������"},
{"Lawful Good", "��������������� ������"},
{"Neutral Good", "���������� ������"},
{"Chaotic Good", "�������� ������"},
{"Lawful Neutral", "��������������� �����������"},
{"Chaotic Neutral", "�������� �����������"},
{"Lawful Evil", "��������������� ����"},
{"Neutral Evil", "���������� ����"},
{"Chaotic Evil", "�������� ����"},
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
			statusbar("������: %1", tips);
		else
			statusbar("������ � ������������ x=%1i, y=%2i", x, y);
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
		button(getwidth() - 110, 10, 100, buttoncancel, "������");
		tick++;
		domodal();
	}
	settimer(0);
}

static void test_grid() {
	struct element {
		const char*		name;
		gender_s		gender;
		alignment_s		alignment;
	};
	static controls::column columns[] = {{Field, "name", "������������", 200},
	{Field, "gender", "���", 128},
	{Field, "alignment", "������������", 200},
	{Field, "alignment.id", "������������ (����)", 200},
	{Check, "check", 0, 28, "��� ������� �������"},
	{Field, "count", "�-��", 48, "���� ������� ��� � �������� �����"},
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
	test.show_grid_lines = true;
	test.read_only = false;
	test.no_change_count = false;
	while(ismodal()) {
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		rc.offset(4 * 2);
		rc.y2 -= button(rc.x2 - 100 + metrics::padding, rc.y2 - draw::texth() - metrics::padding * 3, 100, 0, cmd(buttoncancel), "�����");
		rc.y1 += test.toolbar(rc.x1, rc.y1, rc.width());
		test.view(rc);
		domodal();
	}
}

const char* number_precent(char* result, const char* result_maximum, int value) {
	stringcreator sc;
	sc.prints(result, result_maximum, "%1i%%", value);
	return result;
}

static void test_grid_ref() {
	static controls::column columns[] = {{Field, "name", "������������", 200},
	{Field, "cult_land", "��������������", 128},
	{Field, "cult_land_percent", "�������������� (%)", 200, 0, number_precent},
	{}};
	controls::gridref test(columns, cultivated_land_manager.fields);
	test.add(cultivated_land_manager.get(0));
	test.add(cultivated_land_manager.get(1));
	test.add(cultivated_land_manager.get(1));
	test.no_change_order = false;
	test.show_grid_lines = true;
	test.read_only = false;
	test.no_change_count = false;
	while(ismodal()) {
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		rc.offset(4 * 2);
		rc.y2 -= button(rc.x2 - 100 + metrics::padding, rc.y2 - draw::texth() - metrics::padding * 3, 100, 0, cmd(buttoncancel), "�����");
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
	static widget brands[] = {{Group, 0, 0, 0, 3, 0, 0, elements_left},
	{Group, 0, 0, 0, 3, 0, 0, elements_right},
	{Image, "cat", "art/pictures", 0, 5},
	{}};
	static widget field_group_left[] = {{Field, "name", "���"},
	{Field, "surname", "�������"},
	{Field, "age", "�������"},
	{Field, "gender", "���", 0, 0, 0, 0, 0, "���� ������� ��������� ���������� ����� �� ������"},
	{Field, "alignment", "������������"},
	{}};
	static widget field_group_right[] = {{Button, "button1", "������", 0, 0, 0, 0, 0, 0, KeyEscape, buttoncancel},
	{}};
	static widget field_group[] = {{Group, 0, 0, 0, 8, 0, 0, field_group_left},
	{Group, 0, 0, 0, 4, 0, 0, field_group_right},
	{}};
	static widget elements[] = {{Label, 0, "A **character** who uses a weapon without being proficient with it suffers a [--4] penalty on attack rolls. The character can gain this feat multiple times.Each time the character takes the feat, it applies to a new weapon. A cleric whose deity's favored weapon is a martial weapon and who chooses War as one of his domains receives the Martial Weapon Proficiency feat related to that weapon for free, as well as the [Weapon Focus] feat related to that weapon."},
	{Check, "mark", "������� �������"},
	{Group, 0, "��������� �����", 0, 0, 0, 0, brands},
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
			text(10, 10, szprints(temp, zendof(temp), "�������� ������ %1i, %2i", rc.x1, rc.y1));
		} else if(areb(rc) && hot.key == MouseLeft && hot.pressed)
			drag::begin(1);
		rectf(rc, colors::form);
		rectb(rc, colors::border);
		button(getwidth() - 110, 10, 100, buttoncancel, "������");
		domodal();
	}
}

static void start_menu() {
	struct element {
		const char*		name;
		callback_proc	proc;
		const char*		tips;
	};
	static element element_data[] = {{"����������� ���������", basic_drawing},
	{"������� � ��������", test_grid},
	{"������� ������", test_grid_ref},
	{"�������", test_widget},
	{"��������������", test_drag_drop},
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
	auto s1 = man.strings.add("Test");
	auto s2 = man.strings.add("Test");
	auto s3 = man.strings.add("Test1");
	auto p2 = man.create("pointer");
	man.add(p2, "x", number);
	man.add(p2, "y", number);
	auto p1 = man.create("rect");
	man.add(p1, "x1", number);
	man.add(p1, "y1", number);
	man.add(p1, "x2", number);
	man.add(p1, "y2", number);
}

int main() {
	test_requisit();
	test_array();
	set_light_theme();
	// ������������� ����������
	initialize();
	// �������� ����
	create(-1, -1, 640, 480, WFResize | WFMinmax, 32);
	setcaption("X-Face C++ library");
	start_menu();
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}