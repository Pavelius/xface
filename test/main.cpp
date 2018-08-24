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
	static controls::column columns[] = {{Text, "name", "Наименование", 200},
	{Number, "count", "К-во", 32},
	{}};
	controls::table test(columns);
	while(ismodal()) {
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		rc.offset(4 * 2);
		rc.y2 -= button(rc.x2 - 100, rc.y2 - draw::texth() - metrics::padding * 2, 100 - metrics::padding, 0, cmdpr(buttoncancel), "Назад");
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

static const char* get_text_status(char* result, const char* result_maximum, void* object) {
	szprints(result, result_maximum, "Выбрать закладку '%1'", object);
	return result;
}

static void simple_controls() {
	static const char* elements[] = {"Файл", "Правка", "Вид", "Окна"};
	setfocus(3, true);
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		auto x = 10;
		auto y = 10;
		auto result = tabs(x, y, getwidth() - x*2, false, false, (void**)elements, 0, sizeof(elements) / sizeof(elements[0]), 0, 0,
			get_text, get_text_status, {}); y += 40;
		y += button(x, y, 200, 0, cmdpr(button_accept), "Просто обычная кнопка", "Кнопка, которая отображает подсказку");
		y += button(x, y, 200, 0, cmdpr(test_control), "Тестирование элементов", 0);
		y += button(x, y, 200, Disabled, cmdid(2), "Недоступная кнопка", "Кнопка, которая недоступная для нажатия");
		y += checkbox(x, y, 200, 0, cmdid(3), "Галочка которая выводится и меняет значение чекбокса.");
		y += radio(x, y, 200, 0, cmdid(4), "Первый элемент списка.");
		y += radio(x, y, 200, 0, cmdid(5), "Второй элемент возможного выбора.");
		y += radio(x, y, 200, Checked, cmdid(6), "Третий элемент этого выбора.");
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