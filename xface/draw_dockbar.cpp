#include "bsreq.h"
#include "crt.h"
#include "draw.h"
#include "draw_control.h"
#include "io_plugin.h"
#include "stringbuilder.h"

using namespace draw;
using namespace draw::controls;

static struct dock_info {
	int			current;
	int			size;
} dock_data[DockWorkspace + 1];

bool metrics::show::bottom;
bool metrics::show::top;
bool metrics::show::left;
bool metrics::show::right;

guiplugin* guiplugin::first;

guiplugin::guiplugin(const char* id, dock_s dock) : id(id), dock(dock), next(0) {
	seqlink(this);
}

const guiplugin* guiplugin::find(const char* id) {
	for(auto p = first; p; p = p->next) {
		if(strcmp(p->id, id) == 0)
			return p;
	}
	return 0;
}

static const char* get_control_name(const void* p, char* result, const char* result_maximum, const void* type) {
	result[0] = 0;
	auto pr = ((control*)p)->getlabel(result, result_maximum);
	if(!pr)
		pr = result;
	return pr;
}

aref<control*> getdocked(aref<control*> result, dock_s type) {
	auto ps = result.data;
	auto pe = result.data + result.count;
	for(auto p = guiplugin::first; p; p = p->next) {
		if(p->getcontrol().isdisabled())
			continue;
		if(p->dock == type) {
			if(ps < pe)
				*ps++ = &p->getcontrol();
		}
	}
	result.count = ps - result.data;
	return result;
}

// view control on form
static int paint_control(rect rc, aref<control*> elements, int& current, bool show_toolbar) {
	int y1 = rc.y1;
	if(current >= (int)elements.count)
		current = elements.count - 1;
	auto& ec = *elements[current];
	if(elements.count > 1) {
		int current_hilite = -1;
		const int dy = texth() + 8;
		line(rc.x1, rc.y1 + dy - 1, rc.x2, rc.y1 + dy, colors::border);
		rect rct = {rc.x1, rc.y1, rc.x2, rc.y1 + dy};
		if(tabs(rct, false, false, (void**)elements.data, 0, elements.count,
			current, &current_hilite, get_control_name)) {
			if(current_hilite != -1)
				current = current_hilite;
			else
				current = 0;
		}
		rc.y1 += dy;
	}
	if(show_toolbar && ec.show_toolbar)
		rc.y1 += ec.toolbar(rc.x1, rc.y1, rc.width());
	ec.view(rc);
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
		draw::splitv(rc.x1, rc.y1, e1.size, rc.height(), sx, 64, 400, false);
		client.x1 += e1.size + sx;
		rc.x2 = rc.x1 + e1.size;
		break;
	case DockRight:
		draw::splitv(rc.x2, rc.y1, e1.size, rc.height(), sx, 64, 400, true);
		client.x2 -= e1.size + sx;
		rc.x1 = rc.x2 - e1.size;
		break;
	case DockBottom:
		draw::splith(rc.x1, rc.y2, rc.width(), e1.size, sx + 1, 64, 400, true);
		client.y2 -= e1.size + sx + 1;
		rc.y1 = rc.y2 - e1.size;
		break;
	default:
		return false;
	}
	if(!p2)
		paint_control(rc, p1, e1.current, show_toolbar);
	else if(!p1)
		paint_control(rc, p2, e2.current, show_toolbar);
	else if(id == DockLeft || id == DockRight) {
		draw::splith(rc.x1, rc.y1, rc.width(), e2.size, sx, 64, 400, false);
		paint_control({rc.x1, rc.y1, rc.x2, rc.y1 + e2.size}, p1, e1.current, show_toolbar);
		paint_control({rc.x1, rc.y1 + e2.size + sx, rc.x2, rc.y2}, p2, e2.current, show_toolbar);
	}
	return true;
}

namespace draw {
void dockbar(rect& rc) {
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
}

static struct dockbar_settings_strategy : io::strategy {

	int getindex(const io::reader::node& n, const char* name) const {
		if(!n.parent)
			return -1;
		auto text = n.parent->name;
		while(*name) {
			if(*name++ != *text++)
				return -1;
		}
		return sz2num(text);
	}

	void write(io::writer& file, void* param) override {
		for(auto i = DockLeft; i <= DockWorkspace; i = (dock_s)(i + 1)) {
			char temp[32]; stringbuilder sb(temp);
			sb.add("Dock%1i", i);
			file.open(temp);
			file.set("current", dock_data[i].current);
			file.set("size", dock_data[i].size);
			file.close(temp);
		}
	}

	void set(io::reader::node& n, const char* value) override {
		auto i = getindex(n, "Dock");
		if(i == -1 || i >= sizeof(dock_data) / sizeof(dock_data[0]))
			return;
		if(n == "current")
			dock_data[i].current = sz2num(value);
		else if(n=="size")
			dock_data[i].size = sz2num(value);
	}

	dockbar_settings_strategy() : strategy("dockbar", "settings") {}

} dockbar_settings_strategy_instance;