#include "xface/crt.h"
#include "xface/drawex.h"

using namespace draw;

static void basic_drawing() {
	int tick = 10;
	settimer(100);
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		auto x = 100 + tick % 100;
		auto y = 100 + tick % 200;
		circlef(x, y, 50 + tick % 40, colors::form);
		circle(x, y, 50 + tick % 40, colors::border);
		auto id = input();
		switch(id) {
		case InputTimer:
			tick++;
			break;
		case KeyEscape:
			buttoncancel();
			break;
		default: dodialog(id); break;
		}
	}
	settimer(0);
}

static void button_accept() {
	basic_drawing();
}

static void test_control() {
	static controls::column columns[] = {{Text, "name", "������������", 200},
	{Number, "count", "�-��", 32},
	{}};
	controls::table test(columns);
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::form);
		auto x = 100;
		auto y = 100;
		test.view({x, y, x + 260, y + 200});
		auto id = input();
		switch(id) {
		case KeyEscape:
			buttoncancel();
			break;
		default: dodialog(id); break;
		}
	}
}

static void simple_controls() {
	setfocus(3, true);
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		auto x = 10;
		auto y = 10;
		y += button(x, y, 200, 0, cmdpr(button_accept), "������ ������� ������", "������, ������� ���������� ���������");
		y += button(x, y, 200, 0, cmdpr(test_control), "������������ ���������", 0);
		y += button(x, y, 200, Disabled, cmdid(2), "����������� ������", "������, ������� ����������� ��� �������");
		y += checkbox(x, y, 200, 0, cmdid(3), "������� ������� ��������� � ������ �������� ��������.");
		y += radio(x, y, 200, 0, cmdid(4), "������ ������� ������.");
		y += radio(x, y, 200, 0, cmdid(5), "������ ������� ���������� ������.");
		y += radio(x, y, 200, Checked, cmdid(6), "������ ������� ����� ������.");
		auto id = input();
		switch(id) {
		case KeyEscape:
			return;
		default: dodialog(id); break;
		}
	}
}

void set_dark_theme();
void set_light_theme();

int main() {
	set_light_theme();
	// ������������� ����������
	initialize();
	// �������� ����
	create(-1, -1, 640, 480, WFResize | WFMinmax, 32);
	setcaption("������������ ����������������� ����������");
	simple_controls();
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}