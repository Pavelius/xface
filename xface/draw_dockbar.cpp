#include "crt.h"
#include "draw.h"
#include "draw_control.h"
#include "bsdata.h"

using namespace draw;
using namespace draw::controls;

static struct dock_info {
	int			current;
	int			size;
	bool		visible;
} dock_data[DockWorkspace + 1];

static const char* get_control_name(char* result, const char* result_maximum, void* p) {
	result[0] = 0;
	return ((control*)p)->getlabel(result, result_maximum);
}

static aref<control*> getdocked(aref<control*> result, dock_s type) {
	auto ps = result.data;
	auto pe = result.data + result.count;
	for(auto p = control::plugin::first; p; p = p->next) {
		if(p->element.isdisabled())
			continue;
		if(p->element.dock == type) {
			if(ps < pe)
				*ps++ = &p->element;
		}
	}
	result.count = ps - result.data;
	return result;
}

// view control on form
static int paint_control(rect rc, aref<control*> elements, int& current, bool show_toolbar, unsigned tab_state, int padding) {
	int y1 = rc.y1;
	if(current >= (int)elements.count)
		current = elements.count - 1;
	if(padding == -1)
		padding = metrics::padding;
	auto& ec = *elements[current];
	if(elements.count > 1) {
		int current_hilite = -1;
		const int dy = texth() + 8;
		line(rc.x1, rc.y1 + dy - 1, rc.x2, rc.y1 + dy, colors::border);
		rect rct = {rc.x1, rc.y1, rc.x2, rc.y1 + dy};
		if(tabs(rct, false, false, (void**)pages, 0, count,
			current, &current_hilite, get_control_name)) {
			if(current_hilite != -1)
				current = current_hilite;
			else
				current = 0;
		}
		rc.y1 += dy;
	}
	ec.viewf(rc, show_toolbar);
	return rc.y1 - y1;
}

static bool dock_paint(dock_s id, rect& client, aref<control*> p1, aref<control*> p2) {
	bool show_toolbar = true;
	rect rc = client;
	dock_info& e1 = dock_data[id - DockLeft];
	dock_info& e2 = dock_data[id - DockLeft + 1];
	if(!p1 && !p2)
		return false;
	if(!e1.size)
		e1.size = 200;
	if(!e2.size)
		e2.size = 200;
	const int sx = metrics::padding + 2;
	switch(id) {
	case DockLeft:
		draw::splitv(rc.x1, rc.y1, e1.size, rc.height(), (int)&dock_data[id], sx, 64, 400, false);
		client.x1 += e1.size + sx;
		rc.x2 = rc.x1 + e1.size;
		break;
	case DockRight:
		draw::splitv(rc.x2, rc.y1, e1.size, rc.height(), (int)&dock_data[id], sx, 64, 400, true);
		client.x2 -= e1.size + sx;
		rc.x1 = rc.x2 - e1.size;
		break;
	case DockBottom:
		draw::splith(rc.x1, rc.y2, rc.width(), e1.size, (int)&dock_data[id], sx + 1, 64, 400, true);
		client.y2 -= e1.size + sx + 1;
		rc.y1 = rc.y2 - e1.size;
		break;
	default:
		return false;
	}
	if(!p2)
		paint_control(rc, p1, e1.current, show_toolbar, 0, 0);
	else if(!p1)
		paint_control(rc, p2, e2.current, show_toolbar, 0, 0);
	else if(id == DockLeft || id == DockRight) {
		draw::splith(rc.x1, rc.y1, rc.width(), e2.size, (int)&dock_data[id], sx, 64, 400, false);
		paint_control({rc.x1, rc.y1, rc.x2, rc.y1 + e2.size}, p1, e1.current,
			show_toolbar, 0, 0);
		paint_control({rc.x1, rc.y1 + e2.size + sx, rc.x2, rc.y2}, p2, e2.current,
			show_toolbar, 0, 0);
	}
	return true;
}

static void dockbar(rect& rc) {
	const unsigned max_controls = 65;
	control* p1[max_controls + 1];
	control* p2[max_controls + 1];
	if(metrics::show::left)
		dock_paint(DockLeft, rc, getdocked(p1, DockLeft), getdocked(p2, DockLeftBottom));
	if(metrics::show::right)
		dock_paint(DockRight, rc, getdocked(p1, DockRight), getdocked(p2, DockRightBottom));
	if(metrics::show::bottom)
		dock_paint(DockBottom, rc, getdocked(p1, DockBottom), {});
}