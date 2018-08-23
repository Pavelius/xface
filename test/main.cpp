#include "xface/crt.h"
#include "xface/draw.h"

using namespace draw;

static void basic_drawing() {
	int tick = 10;
	setcaption("Test basic drawing operations");
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
		}
	}
	settimer(0);
}

static void button_accept() {
}

static void simple_controls() {
	setcaption("Test simple controls");
	setfocus(3, true);
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		auto x = 10;
		auto y = 10;
		y += button(x, y, 200, 1, 0, "Просто обычная кнопка", "Кнопка, которая отображает подсказку", button_accept);
		y += button(x, y, 200, 2, Disabled, "Недоступная кнопка", "Кнопка, которая недоступная для нажатия", button_accept);
		y += checkbox(x, y, 200, 3, 0, "Галочка которая выводится и меняет значение чекбокса.");
		y += radio(x, y, 200, 4, 0, "Первый элемент списка.");
		y += radio(x, y, 200, 5, 0, "Второй элемент возможного выбора.");
		y += radio(x, y, 200, 6, Checked, "Третий элемент этого выбора.");
		auto id = input();
		switch(id) {
		case KeyEscape:
			return;
		default: dodialog(id); break;
		}
	}
	settimer(0);
}

void set_dark_theme();
void set_light_theme();

int main() {
	set_light_theme();
	// Инициализация библиотеки
	initialize();
	// Создание окна
	create(-1, -1, 640, 480, WFResize | WFMinmax, 32);
	simple_controls();
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}