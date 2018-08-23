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

void set_dark_theme();
void set_light_theme();

int main() {
	set_dark_theme();
	// Инициализация библиотеки
	initialize();
	// Создание окна
	create(-1, -1, 800, 600, WFResize | WFMinmax, 32);
	basic_drawing();
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}