#include "xface/crt.h"
#include "xface/draw.h"

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
			return;
		default: dodialog(id); break;
		}
	}
	settimer(0);
}

static void button_accept() {
	basic_drawing();
}

static void simple_controls() {
	setfocus(3, true);
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		auto x = 10;
		auto y = 10;
		y += button(x, y, 200, 0, cmdpr(button_accept), "������ ������� ������", "������, ������� ���������� ���������");
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
	set_dark_theme();
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