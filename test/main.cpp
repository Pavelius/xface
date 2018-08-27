#include "xface/crt.h"
#include "xface/draw_control.h"

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

static void test_edit() {
	char temp[260];
	zcpy(temp, "Test edit");
	controls::textedit test(temp, sizeof(temp), true);
	setfocus((int)&test, true);
	auto x = 220;
	auto y = 46;
	test.editing({x, y, x + 200, y + texth() * 5 + metrics::padding * 2});
}

static void test_control() {
	static controls::column columns[] = {{Text, "name", "������������", 200},
	{Number, "count", "�-��", 32},
	{}};
	controls::table test(columns);
	while(ismodal()) {
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		rc.offset(4 * 2);
		rc.y2 -= button(rc.x2 - 100, rc.y2 - draw::texth() - metrics::padding * 2, 100 - metrics::padding, 0, cmdx(buttoncancel), "�����");
		test.view(rc);
		auto id = input();
		switch(id) {
		case KeyEscape:
			buttoncancel();
			break;
		default: dodialog(id); break;
		}
	}
}

static const char* get_text(char* result, const char* result_maximum, void* object) {
	return (const char*)object;
}

static int button(int x, int y, int width, void(*proc)(), const char* title, const char* tips = 0) {
	auto result = button(x, y, width, 0, cmdx(proc), title, tips);
	rect rc = {x, y, x + width, y + texth() + metrics::padding * 2};
	if(areb(rc)) {
		if(tips)
			statusbar("������: %1", tips);
		else
			statusbar("������ � ������������ x=%1i, y=%2i", x, y);
	}
	return result;
}

static void simple_controls() {
	struct cmdwf : cmdfd {
		int getid() const override { return id;}
		void execute(const rect& rc) const override {
		}
		bool choose(bool run) const override { return true; }
		bool increment(int step, bool run) const override { return true; }
		cmdwf(int id) : id(id) {}
	private:
		int	id;
	};
	static const char* elements[] = {"����", "������", "���", "����"};
	setfocus(3, true);
	int current_hilite;
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		statusbardw();
		auto x = 10; auto y = 10;
		auto result = tabs(x, y, getwidth() - x*2, false, false, (void**)elements, 0, sizeof(elements) / sizeof(elements[0]), 0, &current_hilite,
			get_text, {}); y += 40;
		if(current_hilite != -1)
			statusbar("������� �������� '%1'", elements[current_hilite]);
		y += button(x, y, 200, button_accept, "������ ������� ������", "������, ������� ���������� ���������");
		y += button(x, y, 200, test_control, "������������ ���������");
		y += button(x, y, 200, test_edit, "�������������� ������");
		y += button(x, y, 200, Disabled, cmdx(2), "����������� ������", "������, ������� ����������� ��� �������");
		y += checkbox(x, y, 200, 0, cmdx(3), "������� ������� ��������� � ������ �������� ��������.");
		y += field(x, y, 200, TextSingleLine, cmdwf(8), "�����", "��� ��������� ������ ��� ��������������", "���", 80);
		y += field(x, y, 200, TextSingleLine, cmdwf(9), "34", "��� �������� ����", "����������", 80);
		y += radio(x, y, 200, 0, cmdx(4), "������ ������� ������.");
		y += radio(x, y, 200, 0, cmdx(5), "������ ������� ���������� ������.");
		y += radio(x, y, 200, Checked, cmdx(6), "������ ������� ����� ������.");
		auto id = input();
		switch(id) {
		case KeyEscape: return;
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
	setcaption("X-Face C++ library");
	simple_controls();
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}