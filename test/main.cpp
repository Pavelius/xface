#include "xface/bsdata.h"
#include "xface/bsreq.h"
#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/draw_control.h"
#include "xface/variable.h"

using namespace draw;

static unsigned radio_button = 2;
static unsigned check_button = 0;

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

static void button_accept() {
	basic_drawing();
}

static void test_control() {
	static controls::column columns[] = {{Text, "name", "������������", 200},
	{Field, "count", "�-��", 32},
	{}};
	controls::table test(columns);
	while(ismodal()) {
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		rc.offset(4 * 2);
		rc.y2 -= button(rc.x2 - 100, rc.y2 - draw::texth() - metrics::padding * 3, 100 - metrics::padding, 0, cmd(buttoncancel), "�����");
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
	};
	static bsreq element_type[] = {
		BSREQ(element, mark, number_type),
		BSREQ(element, radio, number_type),
		BSREQ(element, name, text_type),
		BSREQ(element, surname, text_type),
		BSREQ(element, age, number_type),
		BSREQ(element, gender, gender_type),
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
	static widget field_group_left[] = {{Field, "name", "���"},
	{Field, "surname", "�������"},
	{Field, "age", "�������"},
	{Field, "gender", "���", 0, 0, 0, 0, 0, "���� ������� ��������� ���������� ����� �� ������"},
	{}};
	static widget field_group_right[] = {{Button, "button1", "������", 0, 0, 0, 0, 0, 0, KeyEscape, buttoncancel},
	{}};
	static widget field_group[] = {{Group, 0, 0, 0, 8, 0, 0, field_group_left},
	{Group, 0, 0, 0, 4, 0, 0, field_group_right},
	{}};
	static widget elements[] = {{Text, 0, "A **character** who uses a weapon without being proficient with it suffers a [--4] penalty on attack rolls. The character can gain this feat multiple times.Each time the character takes the feat, it applies to a new weapon. A cleric whose deity's favored weapon is a martial weapon and who chooses War as one of his domains receives the Martial Weapon Proficiency feat related to that weapon for free, as well as the [Weapon Focus] feat related to that weapon."},
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

static void disabled_button() {
}

static void simple_controls() {
	static const char* elements[] = {"����", "������", "���", "����"};
	setfocus(3, true);
	int current_hilite;
	const char* t1 = "���� 1";
	char t2 = 20;
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		statusbardw();
		auto x = 10; auto y = 10;
		auto result = tabs(x, y, getwidth() - x * 2, false, false, (void**)elements, 0, sizeof(elements) / sizeof(elements[0]), 0, &current_hilite,
			get_text, {}); y += 40;
		if(current_hilite != -1)
			statusbar("������� �������� '%1'", elements[current_hilite]);
		y += button(x, y, 200, button_accept, "����������� ���������", "������, ������� ���������� ���������");
		y += button(x, y, 200, test_control, "������� ����������");
		y += button(x, y, 200, test_widget, "�������");
		y += button(x, y, 200, Disabled, cmd(disabled_button), "����������� ������", "������, ������� ����������� ��� �������");
		domodal();
	}
}

static void test_array() {
	amem a1(sizeof(testinfo));
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

static bool szpmatch(const char* text, const char* s, const char* s2) {
	while(true) {
		register const char* d = text;
		while(s < s2) {
			if(*d == 0)
				return false;
			unsigned char c = *s;
			if(c == '?') {
				s++;
				d++;
			} else if(c == '*') {
				s++;
				if(s == s2)
					return true;
				while(*d) {
					if(*d == *s)
						break;
					d++;
				}
			} else {
				if(*d++ != *s++)
					return false;
			}
		}
		return true;
	}
}

int main() {
	test_array();
	set_light_theme();
	// ������������� ����������
	initialize();
	// �������� ����
	create(-1, -1, 640, 480, WFResize | WFMinmax, 32);
	setcaption("X-Face C++ library");
	simple_controls();
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}