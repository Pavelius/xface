#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

void picker::view(const rect& rcorigin) {
	view_rect = rcorigin;
	rect rc = rcorigin;
	pixels_per_width = rc.width() - 1;
	correction_width();
	control::view(rc);
	current_rect.clear();
	rc.x1 += 1; rc.y1 += 1; // Чтобы был отступ для первой строки
	if(!pixels_per_line)
		pixels_per_line = getrowheight();
	elements_per_line = rc.width() / pixels_per_column;
	if(!elements_per_line)
		return;
	//current_hilite = -1;
	lines_per_page = getlinesperpage(rc.height());
	correction();
	auto maximum = getmaximum();
	auto maximum_height = getmaximum() / elements_per_line;
	auto maximum_width = getmaximumwidth();
	if(!pixels_per_line)
		return;
	auto enable_scrollv = maximum_height > lines_per_page;
	if(true) {
		draw::state push; setclip(rc);
		auto c = 0;
		auto x = rc.x1;
		auto y = rc.y1;
		for(auto i = origin; i < maximum; i++) {
			if(c >= elements_per_line) {
				x = rc.x1;
				y += pixels_per_line;
				c = 0;
				if(y > rc.y2)
					break;
			}
			rect rc = {x, y, x + pixels_per_column, y + pixels_per_line};
			if(ishilite(rc)) {
				//current_hilite = i;
				//if(hot.key == MouseLeft && hot.pressed)
				//	execute(choose_mouse, (int)static_cast<picker*>(this));
			}
			if(show_grid_lines) {
				rectb(rc, colors::border);
				rc.x1 += 1;
				rc.y1 += 1;
			}
			row(rc, i);
			c++;
			x += pixels_per_column;
		}
	}
	if(enable_scrollv)
		draw::scrollv({rc.x2 - metrics::scroll, rc.y1, rc.x2, rc.y2},
			origin, lines_per_page*elements_per_line, maximum, isfocused());
}

void picker::ensurevisible() {
	correction();
	if(!elements_per_line)
		return;
	if(current < origin)
		origin = current;
	if(current > origin + (lines_per_page - 1)*elements_per_line)
		origin = current - (lines_per_page - 1)*elements_per_line;
	origin = (origin / elements_per_line) * elements_per_line;
}

void picker::mousewheel(unsigned id, point position, int step) {
	origin += step * elements_per_line;
	auto maximum = getmaximum();
	if(origin > maximum - lines_per_page*elements_per_line)
		origin = maximum - lines_per_page * elements_per_line;
	if(origin < 0)
		origin = 0;
}

bool picker::keyinput(unsigned id) {
	int m;
	switch(id) {
	case KeyUp:
		current -= elements_per_line;
		ensurevisible();
		break;
	case KeyRight:
		current++;
		ensurevisible();
		break;
	case KeyLeft:
		current--;
		ensurevisible();
		break;
	case KeyDown:
		current += elements_per_line;
		ensurevisible();
		break;
	case KeyHome:
		if(current == 0)
			break;
		current = 0;
		ensurevisible();
		break;
	case KeyEnd:
		m = getmaximum();
		if(current == m - 1)
			break;
		current = m - 1;
		ensurevisible();
		break;
	case KeyPageUp:
		m = origin + (current % elements_per_line);
		if(current != m)
			current = m;
		else
			current -= (lines_per_page - 1)*elements_per_line;
		ensurevisible();
		break;
	case KeyPageDown:
		m = origin + (lines_per_page - 1)*elements_per_line + current % elements_per_line;
		if(current != m)
			current = m;
		else
			current += (lines_per_page - 1)*elements_per_line;
		ensurevisible();
		break;
	default:
		return list::keyinput(id);
	}
	return true;
}