#include "crt.h"
#include "drawex.h"

using namespace draw::controls;

static char		search_text[32];
static unsigned	search_time;

list::list() : origin(0), current(0), current_hilite(-1),
maximum_width(0), origin_width(0),
lines_per_page(0), pixels_per_line(0),
show_grid_lines(false),
show_selection(true),
hilite_odd_lines(true) {
}

void list::ensurevisible() {
	if(current < origin)
		origin = current;
	if(current > origin + lines_per_page - 1)
		origin = current - lines_per_page + 1;
}

void list::select(int index) {
	current = index;
	ensurevisible();
}

void list::correction() {
	auto maximum = getmaximum();
	if(current >= maximum)
		current = maximum - 1;
	if(current < 0)
		current = 0;
	if(lines_per_page) {
		if(origin > maximum - lines_per_page)
			origin = maximum - lines_per_page;
		if(origin < 0)
			origin = 0;
	}
}

void list::hilight(rect rc) const {
	auto focused = isfocused();
	const color c1 = focused ? colors::edit : colors::edit.mix(colors::window, 192);
	rc.y2--; rc.x2--;
	if(!focused)
		rc.y2--;
	rectf(rc, c1);
	rectb(rc, c1);
	if(focused)
		rectx(rc, colors::text.mix(colors::form, 200));
}

void list::rowhilite(rect rc, int index) const {
	if(show_selection) {
		area(rc);
		if(index == current)
			hilight(rc);
		else if(index == current_hilite)
			rectf({rc.x1, rc.y1, rc.x2, rc.y2 - 1}, colors::edit.mix(colors::window, 96));
		else if(hilite_odd_lines) {
			if(index & 1)
				rectf({rc.x1, rc.y1, rc.x2, rc.y2 - 1}, colors::edit, 64);
		}
	} else if(hilite_odd_lines) {
		if(index & 1)
			rectf({rc.x1, rc.y1, rc.x2, rc.y2 - 1}, colors::edit, 64);
	}
}

void list::row(rect rc, int index) const {
	char temp[260]; temp[0] = 0;
	rowhilite(rc, index);
	auto p = getname(temp, temp + sizeof(temp) / sizeof(temp[0]) - 1, index, 0);
	if(p)
		draw::textc(rc.x1 + 4, rc.y1 + 4, rc.width() - 4 * 2, p);
}

int	list::getrowheight() {
	return texth() + 8;
}

void list::view(rect rcorigin) {
	control::view(rcorigin);
	rect rc = rcorigin; rc.offset(1, 1);
	if(!pixels_per_line)
		pixels_per_line = getrowheight();
	current_hilite = -1;
	auto maximum = getmaximum();
	if(!pixels_per_line)
		return;
	rect scroll = {0};
	rect scrollh = {0};
	lines_per_page = rcorigin.height() / pixels_per_line;
	correction();
	if(maximum > lines_per_page)
		scroll.set(rcorigin.x2 - metrics::scroll, rcorigin.y1, rcorigin.x2, rcorigin.y2);
	if(maximum_width > rc.width())
		scrollh.set(rcorigin.x1, rcorigin.y2 - metrics::scroll, rcorigin.x2, rcorigin.y2);
	int rk = hot::key&CommandMask;
	if(draw::areb(rc)) {
		if(hot::mouse.y > rc.y1 && hot::mouse.y <= rc.y1 + pixels_per_line * (maximum - origin)) {
			if(!scroll.width() || hot::mouse.x < scroll.x1)
				current_hilite = origin + (hot::mouse.y - rc.y1) / pixels_per_line;
		}
		// Mouse select
		if(hot::pressed && (rk == MouseLeft || rk == MouseRight)) {
			if(current_hilite != -1)
				select(current_hilite);
		}
	}
	if(true) {
		draw::state push;
		setclip(rcorigin);
		int x1 = rc.x1;
		int y1 = rc.y1;
		int rw = rc.x2 - x1 + 1;
		int ix = origin;
		while(true) {
			if(y1 >= rc.y2)
				break;
			if(ix >= maximum)
				break;
			rect rcm = {x1 - origin_width, y1, rc.x1 + rw, y1 + pixels_per_line};
			if(show_grid_lines)
				line(rcm.x1, rcm.y2 - 1, rcm.x2, rcm.y2 - 1, colors::border);
			row(rcm, ix);
			y1 += pixels_per_line;
			ix++;
		}
	}
	if(scroll)
		draw::scrollv((int)this, scroll, origin, lines_per_page, maximum, isfocused());
	if(scrollh)
		draw::scrollh((int)this, scrollh, origin_width, rc.width(), maximum_width, isfocused());
}

void list::keyup() {
	current--;
	correction();
	ensurevisible();
}

void list::keydown() {
	current++;
	correction();
	ensurevisible();
}

void list::keyhome() {
	if(current == 0)
		return;
	current = 0;
	correction();
	ensurevisible();
}

void list::keyend() {
	auto maximum = getmaximum();
	if(current == maximum - 1)
		return;
	current = maximum - 1;
	correction();
	ensurevisible();
}

void list::keypageup() {
	if(current != origin)
		current = origin;
	else
		current -= lines_per_page - 1;
	correction();
	ensurevisible();
}

void list::keypagedown() {
	if(current != (origin + lines_per_page - 1))
		current = (origin + lines_per_page - 1);
	else
		current += lines_per_page - 1;
	correction();
	ensurevisible();
}

void list::mouseleftdbl(point position) {
	keyenter();
}

void list::keyenter() {
}

void list::mousewheel(point position, int step) {
	origin += step;
	auto maximum = getmaximum();
	if(origin > maximum - lines_per_page)
		origin = maximum - lines_per_page;
	if(origin < 0)
		origin = 0;
}

int list::find(int line, int column, const char* value, int lenght) const {
	if(line < 0)
		line = getline() + 1;
	if(column == -1)
		column = getcolumn();
	if(lenght == -1)
		lenght = zlen(value);
	auto m = getmaximum();
	while(line < m) {
		char temp[260]; temp[0] = 0;
		auto p = getname(temp, temp + sizeof(temp) - 1, line, column);
		if(p && szcmpi(p, value, lenght)==0)
			return line;
		line++;
	}
	return -1;
}

void list::keysymbol(int symbol) {
	if(!symbol || symbol < 0x20)
		return;
	auto time_clock = clock();
	if(!search_time || (time_clock - search_time) >1500)
		search_text[0] = 0;
	search_time = time_clock;
	char* p = zend(search_text);
	szput(&p, hot::param); p[0] = 0;
	int i1 = find(-1, -1, search_text);
	if(i1 != -1) {
		current = i1;
		correction();
		ensurevisible();
	}
}