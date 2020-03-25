#include "crt.h"
#include "draw_control.h"

using namespace draw::controls;

static char		search_text[32];
static unsigned	search_time;

void list::ensurevisible() {
	correction();
	if(current < origin)
		origin = current;
	if(current > origin + lines_per_page - 1)
		origin = current - lines_per_page + 1;
}

void list::select(int index, int column) {
	current = index;
	ensurevisible();
}

void list::correction_width() {
	auto maximum_width = getmaximumwidth();
	if(pixels_per_width) {
		if(origin_width > maximum_width - pixels_per_width)
			origin_width = maximum_width - pixels_per_width;
		if(origin_width < 0)
			origin_width = 0;
	}
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

void list::hilight(const rect& rc, const rect* p_current_rect) const {
	if(!p_current_rect)
		p_current_rect = &rc;
	auto focused = isfocused();
	const color c1 = focused ? colors::edit : colors::edit.mix(colors::window, 192);
	rect r1 = {rc.x1, rc.y1, rc.x2-1, rc.y2-1};
	rectf(r1, c1); rectb(r1, c1);
	if(focused)
		rectx(r1, colors::text.mix(colors::form, 200));
	const_cast<list*>(this)->current_rect = *p_current_rect;
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

void list::row(const rect& rc, int index) {
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

static void list_mouse_select() {
	auto p = (list*)draw::hot.param;
	p->select(p->current_hilite, p->getcolumn());
}

void list::mouseselect(int id, bool pressed) {
	if(current_hilite < 0)
		return;
	if(pressed)
		draw::execute(list_mouse_select, (int)this);
}

bool list::isopen(int index) const {
	return (index < getmaximum() - 1) ? (getlevel(index + 1) > getlevel(index)) : false;
}

void list::treemark(rect rc, int index, int level) const {
	if(!isgroup(index))
		return;
	color c1 = colors::text;
	auto count = getmaximum();
	auto isopen = list::isopen(index);
	int x = rc.x1 + rc.width() / 2;
	int y = rc.y1 + rc.height() / 2 - 1;
	areas a = area(rc);
	if(a == AreaHilitedPressed) {
		if(hot.key == MouseLeft)
			execute((fncmd)&list::treemarking, index);
	}
	circle(x, y, 6, c1);
	line(x - 4, y, x + 4, y, c1);
	if(!isopen)
		line(x, y - 4, x, y + 4, c1);
}

void list::view(const rect& rcorigin) {
	view_rect = rcorigin;
	rect rc = rcorigin;
	pixels_per_width = rc.width() - 1;
	correction_width();
	if(show_header)
		rc.y1 += rowheader(rc);
	control::view(rc);
	current_rect.clear();
	rc.x1 += 1; rc.y1 += 1; // Чтобы был отступ для первой строки
	if(!pixels_per_line)
		pixels_per_line = getrowheight();
	current_hilite = -1;
	lines_per_page = getlinesperpage(rc.height());
	correction();
	auto maximum = getmaximum();
	auto maximum_width = getmaximumwidth();
	if(!pixels_per_line)
		return;
	auto enable_scrollv = maximum > lines_per_page;
	auto enable_scrollh = maximum_width > pixels_per_width;
	int rk = hot.key&CommandMask;
	if(draw::areb(rc)) {
		if(hot.mouse.y > rc.y1 && hot.mouse.y <= rc.y1 + pixels_per_line * (maximum - origin)) {
			if((!enable_scrollv || hot.mouse.x < rc.x2 - metrics::scroll)
				&& (!enable_scrollh || hot.mouse.y < rc.y2 - metrics::scroll))
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
	if(enable_scrollv)
		draw::scrollv({rc.x2 - metrics::scroll, rc.y1, rc.x2, rc.y2},
			origin, lines_per_page, maximum, isfocused());
	if(enable_scrollh)
		draw::scrollh({rc.x1, rc.y2 - metrics::scroll, rc.x2, rc.y2},
			origin_width, pixels_per_width, maximum_width, isfocused());
	if(drop_shadow) {
		rectf({rcorigin.x2 + 1, rcorigin.y1 + 4, rcorigin.x2 + 5, rcorigin.y2}, colors::black, 64);
		rectf({rcorigin.x1 + 4, rcorigin.y2, rcorigin.x2 + 5, rcorigin.y2 + 5}, colors::black, 64);
	}
}

void list::redraw() {
	list::view(view_rect);
}

bool list::keyinput(unsigned id) {
	int m;
	switch(id) {
	case KeyUp:
		current--;
		ensurevisible();
		break;
	case KeyDown:
		current++;
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
		if(current != origin)
			current = origin;
		else
			current -= lines_per_page - 1;
		ensurevisible();
		break;
	case KeyPageDown:
		if(current != (origin + lines_per_page - 1))
			current = (origin + lines_per_page - 1);
		else
			current += lines_per_page - 1;
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
	default:
		control::mouseinput(id, position);
		break;
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