#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/draw_control.h"

using namespace draw;

struct cmdwf : cmdfd {

	static rect		current_rect;
	static cmdwf	current;

	static void test_edit() {
		controls::textedit test(current.source, current.maximum, true);
		setfocus((int)&test, true);
		test.editing(current_rect);
	}

	int getid() const override { return (int)source; }
	void execute(const rect& rc) const override {
		current = *this;
		current_rect = rc;
		draw::execute(test_edit);
	}
	bool choose(bool run) const override { return true; }
	bool increment(int step, bool run) const override { return true; }
	cmdwf() = default;
	cmdwf(aref<char> source) : source(source.data), maximum(source.count)  {}
private:
	char*		source;
	unsigned	maximum;
};
rect cmdwf::current_rect;
cmdwf cmdwf::current;

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
	static const char* elements[] = {"����", "������", "���", "����"};
	setfocus(3, true);
	int current_hilite;
	char t1[100] = "���� 1";
	char t2[200] = "���� 2";
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
		//y += button(x, y, 200, test_edit, "�������������� ������");
		y += button(x, y, 200, Disabled, cmdx(2), "����������� ������", "������, ������� ����������� ��� �������");
		y += checkbox(x, y, 200, 0, cmdx(3), "������� ������� ��������� � ������ �������� ��������.");
		y += field(x, y, 400, TextSingleLine, cmdwf(t1), t1, "��� ��������� ������ ��� ��������������", "���", 80);
		y += field(x, y, 400, TextSingleLine, cmdwf(t2), t2, "��� �������� ����", "����������", 80);
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