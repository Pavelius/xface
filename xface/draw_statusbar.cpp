#include "crt.h"
#include "draw.h"
#include "stringcreator.h"

using namespace draw;

static rect	statusbar_rect;
static char	statusbar_text[512];

void draw::statusbarv(const char* format, const char* format_param) {
	szprintvs(statusbar_text, statusbar_text + sizeof(statusbar_text) - 1, format, format_param);
}

void draw::statusbar(const char* format, ...) {
	statusbarv(format, xva_start(format));
}

int draw::statusbardw() {
	if(!font)
		return 0;
	int dx = font->height + 6;
	statusbar_rect.set(0, getheight() - dx, getwidth(), getheight());
	gradv(statusbar_rect, colors::button.lighten(), colors::button.darken());
	line(statusbar_rect.x1, statusbar_rect.y1,
		statusbar_rect.x2, statusbar_rect.y1, colors::border);
	statusbar_rect.offset(4, 3);
	return dx;
}

static struct statusbar_plugin : draw::plugin {

	void before() override {
		statusbar_rect.clear();
		statusbar_text[0] = 0;
	}

	void after() override {
		// ¬ задачи этого расширени€ вывести короткое сообщение статуса
		// в специальную область под названием 'область статуса'.
		// “екст сообщени€ можт определ€тьс€ в процессе рендеринга
		// и он должен наложитьс€ на экран в указанную область
		// если нет больше никаких других команд
		if(!statusbar_rect)
			return;
		if(statusbar_text[0])
			draw::text(statusbar_rect, statusbar_text);
	}

} plugin_instance;