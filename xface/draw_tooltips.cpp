#include "crt.h"
#include "draw.h"

rect tooltips_rect;
char tooltips_text[4096];

void draw::tooltipsv(int x, int y, int width, const char* format, const char* format_param) {
	tooltips_rect = {x, y, x + width, y};
	tooltips_text[0] = 0;
	if(hot::key != InputIdle)
		return;
	szprintvs(tooltips_text, zendof(tooltips_text), format, format_param);
}

void draw::tooltips(int x, int y, int width, const char* format, ...) {
	tooltipsv(x, y, width, format, xva_start(format));
}

void draw::tooltips(const char* format, ...) {
	tooltipsv(hot::mouse.x, hot::mouse.y + 24, 400, format, xva_start(format));
}

static struct tooltips_renderplugin : draw::renderplugin {

	void after() override {
		if(!tooltips_text[0] || !tooltips_rect)
			return;
		draw::state push;
		draw::font = metrics::font;
		if(draw::font) {
			rect rc = tooltips_rect;
			if(tooltips_rect.y1 == tooltips_rect.y2)
				draw::textf(rc, tooltips_text);
			rc = rc - metrics::padding;
			// Correct border
			int height = draw::getheight();
			int width = draw::getwidth();
			if(rc.y2 >= height)
				rc.move(0, height - 2 - rc.y2);
			if(rc.x2 >= width)
				rc.move(width - 2 - rc.x2, 0);
			// Show background
			draw::rectf(rc, colors::tips::back);
			draw::rectb(rc, colors::border);
			rc = rc + metrics::padding;
			// Show text
			draw::fore = colors::tips::text;
			draw::textf(rc.x1, rc.y1, rc.width(), tooltips_text);
		}
		tooltips_rect.clear();
		tooltips_text[0] = 0;
	}

} input_plugin_instance;