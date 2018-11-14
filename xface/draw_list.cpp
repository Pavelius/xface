#include "crt.h"
#include "draw_control.h"

using namespace draw::controls;

static char		search_text[32];
static unsigned	search_time;

void list::ensurevisible() {
	if(current < origin)
		origin = current;
	if(current > origin + lines_per_page - 1)
		origin = current - lines_per_page + 1;
}

void list::select(int index, int column) {
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

void list::hilight(const rect& rc) const {
	auto focused = isfocused();
	const color c1 = focused ? colors::edit : colors::edit.mix(colors::window, 192);
	rect r1 = {rc.x1, rc.y1, rc.x2-1, rc.y2-1};
	rectf(r1, c1); rectb(r1, c1);
	if(focused)
		rectx(r1, colors::text.mix(colors::form, 200));
	const_cast<list*>(this)->current_rect = rc;
}

void list::rowhilite(const rect& rc, int index) const {
	if(show_selection) {
		area({rc.x1, rc.y1, rc.x2 - 1, rc.y2 - 1});
		if(index == current)
			hilight(rc);
		else if(index == current_hilite)
			rectf(rc, colors::edit.mix(colors::window, 96));
	}
}

void list::row(const rect& rc, int index) const {
	char temp[260]; temp[0] = 0;
	rowhilite(rc, index);
	auto p = getname(temp, temp + sizeof(temp) / sizeof(temp[0]) - 1, index, 0);
	if(p)
		draw::textc(rc.x1 + 4, rc.y1 + 4, rc.width() - 4 * 2, p);
}

int	list::getrowheight() {
	return texth() + 8;
}

void list::mousehiliting(const rect& screen, point mouse) {
	current_hilite = origin + (mouse.y - screen.y1) / pixels_per_line;
}

void list::mouseselect(int id, bool pressed) {
	if(current_hilite < 0)
		return;
	if(pressed)
		select(current_hilite, getcolumn());
}

void list::view(const rect& rcorigin) {
	current_rect.clear();
	rect rc = rcorigin;
	control::view(rcorigin);
	rc.x1 += 1; rc.y1 += 1; // Чтобы был отступ для первой строки
	if(!pixels_per_line)
		pixels_per_line = getrowheight();
	current_hilite = -1;
	auto maximum = getmaximum();
	if(!pixels_per_line)
		return;
	rect scroll = {0};
	rect scrollh = {0};
	lines_per_page = getlinesperpage(rc.height());
	correction();
	if(maximum > lines_per_page)
		scroll.set(rc.x2 - metrics::scroll, rc.y1, rc.x2, rc.y2);
	auto maximum_width = getmaximumwidth();
	if(maximum_width > rc.width())
		scrollh.set(rc.x1, rc.y2 - metrics::scroll, rc.x2, rc.y2);
	int rk = hot.key&CommandMask;
	if(draw::areb(rc)) {
		if(hot.mouse.y > rc.y1 && hot.mouse.y <= rc.y1 + pixels_per_line * (maximum - origin)) {
			if(!scroll.width() || hot.mouse.x < scroll.x1)
				mousehiliting(rc, hot.mouse);
		}
		if(rk == MouseLeft || rk == MouseRight)
			mouseselect(rk, hot.pressed);
	}
	if(true) {
		draw::state push;
		setclip(rc);
		auto x1 = rc.x1 - origin_width, y1 = rc.y1;
		auto x2 = rc.x2;
		auto ix = origin;
		auto hl = colors::border.mix(colors::window, 12);
		while(true) {
			if(y1 >= rc.y2)
				break;
			if(ix >= maximum)
				break;
			rect rcm = {x1, y1, x2, y1 + pixels_per_line};
			if(hilite_odd_lines) {
				if(ix & 1)
					rectf(rcm, hl);
			}
			if(show_grid_lines)
				line(rc.x1, rcm.y2 - 1, rc.x2, rcm.y2 - 1, colors::border);
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

bool list::keyinput(unsigned id) {
	int m;
	switch(id) {
	case KeyUp:
		current--;
		correction();
		ensurevisible();
		break;
	case KeyDown:
		current++;
		correction();
		ensurevisible();
		break;
	case KeyHome:
		if(current == 0)
			break;
		current = 0;
		correction();
		ensurevisible();
		break;
	case KeyEnd:
		m = getmaximum();
		if(current == m - 1)
			break;
		current = m - 1;
		correction();
		ensurevisible();
		break;
	case KeyPageUp:
		if(current != origin)
			current = origin;
		else
			current -= lines_per_page - 1;
		correction();
		ensurevisible();
		break;
	case KeyPageDown:
		if(current != (origin + lines_per_page - 1))
			current = (origin + lines_per_page - 1);
		else
			current += lines_per_page - 1;
		correction();
		ensurevisible();
		break;
	case InputSymbol:
		if(hot.key >= 0x20) {
			auto time_clock = clock();
			if(!search_time || (time_clock - search_time) > 1500)
				search_text[0] = 0;
			search_time = time_clock;
			char* p = zend(search_text);
			szput(&p, hot.param); p[0] = 0;
			int i1 = find(-1, -1, search_text);
			if(i1 != -1) {
				current = i1;
				correction();
				ensurevisible();
			}
		}
		break;
	default:
		return control::keyinput(id);
	}
	return true;
}

void list::mouseinput(unsigned id, point position) {
	switch(id) {
	case MouseLeftDBL:
		keyinput(KeyEnter);
		break;
	default: control::mouseinput(id, position);
	}
}

void list::mousewheel(unsigned id, point position, int step) {
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
		if(p && szcmpi(p, value, lenght) == 0)
			return line;
		line++;
	}
	return -1;
}