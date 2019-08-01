#include "crt.h"
#include "draw.h"

int draw::clipart(int x, int y, int width, unsigned flags, const char* string) {
	if(!string || string[0] != ':')
		return 0;
	auto p = string + 1;
	if(strcmp(p, "check") == 0) {
		rect rc = {x, y, x + 14, y + 14};
		rectf(rc, colors::window);
		color b = getcolor(colors::border, flags);
		line(rc.x1, rc.y1, rc.x2, rc.y1, b);
		line(rc.x2, rc.y1, rc.x2, rc.y2, b);
		line(rc.x2, rc.y2, rc.x1, rc.y2, b);
		line(rc.x1, rc.y2, rc.x1, rc.y1, b);
		if(flags&Checked) {
			state push;
			fore = getcolor(rc, colors::text, colors::text.mix(colors::active, 32), flags);
			linw = 1.3f;
			line(x + 3, y + 6, x + 6, y + 10);
			line(x + 6, y + 10, x + 11, y + 3);
		}
		return rc.height();
	} else if(strcmp(p, "radio") == 0) {
		rect rc = {x, y, x + 16, y + 16};
		circlef(x + 7, y + 6, 6, colors::window);
		circle(x + 7, y + 6, 6, getcolor(colors::border, flags));
		if(flags&Checked) {
			color b = getcolor(rc, colors::text, colors::text.mix(colors::active, 32), flags);
			circlef(x + 7, y + 6, 2, b);
			circle(x + 7, y + 6, 2, b);
		}
		return rc.height();
	} else if(strcmp(p, "dropdown") == 0) {
		static point clipart_dropdown[] = {{-2, -1}, {-1, -1}, {0, -1}, {1, -1}, {2, -1},
		{-1, 0}, {0, 0}, {1, 0},
		{0, 1}
		};
		state push;
		rect rc = {x, y, x + width, y + 16};
		x += width / 2;
		y += 12;
		fore = getcolor(rc, colors::black, colors::blue, flags);
		for(auto e : clipart_dropdown)
			pixel(x + e.x, y + e.y);
		return rc.height();
	} else if(strcmp(p, "tree") == 0)
		circle(x + 7, y + 6, 6, getcolor(colors::border, flags));
	return 0;
}