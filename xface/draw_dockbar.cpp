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
const unsigned max_controls = 65;

control::plugin* control::plugin::first;

control::plugin::plugin(const char* id, dock_s dock) : id(id), dock(dock), visible(true), next(0) {
	seqlink(this);
}

const control::plugin* control::plugin::find(const char* id) {
	for(auto p = first; p; p = p->next) {
		if(strcmp(p->id, id) == 0)
			return p;
	}
	return 0;
}

static const char* get_control_name(const void* p, stringbuilder& sb) {
	auto pr = ((control*)p)->getlabel(sb);
	if(!pr)
		pr = sb;
	return pr;
}

controla getdocked(control** result, unsigned count, dock_s type) {
	auto ps = result;
	auto pe = result + count;
	for(auto p = control::plugin::first; p; p = p->next) {
		if(!p->visible)
			continue;
		auto pc = p->getcontrol();
		if(!pc)
			continue;
		if(pc->isdisabled())
			continue;
		if(p->dock == type) {
			if(ps < pe)
				*ps++ = pc;
		}
	}
	return controla(result, ps);
}

// view control on form
static int paint_control(rect rc, controla elements, int& current, bool show_toolbar) {
	int y1 = rc.y1;
	if(current >= (int)elements.size())
		current = elements.size() - 1;
	auto& ec = *(elements.begin() + current);
	if(elements.size() > 1) {
		int current_hilite = -1;
		const int dy = texth() + 8;
		line(rc.x1, rc.y1 + dy - 1, rc.x2, rc.y1 + dy, colors::border);
		rect rct = {rc.x1, rc.y1, rc.x2, rc.y1 + dy};
		if(tabs(rct, false, false, (void**)elements.begin(), 0, elements.size(),
			current, &current_hilite, get_control_name)) {
			if(current_hilite != -1)
				current = current_hilite;
			else
				current = 0;
		}
		rc.y1 += dy;
	}
	if(show_toolbar && ec->show_toolbar)
		rc.y1 += ec->toolbar(rc.x1, rc.y1, rc.width());
	ec->view(rc);
	return rc.y1 - y1;
}

static bool dock_paint(dock_s id, rect& client, controla p1, controla p2) {
	bool show_toolbar = true;
	rect rc = client;
	dock_info& e1 = dock_data[id - DockLeft];
	dock_info& e2 = dock_data[id - DockLeft + 1];
	if(p1.size() == 0 && p2.size() == 0)
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
	if(p2.size()==0)
		paint_control(rc, p1, e1.current, show_toolbar);
	else if(p1.size() == 0)
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
	control* p1[max_controls];
	control* p2[max_controls];
	if(metrics::show::left)
		dock_paint(DockLeft, rc, getdocked(p1, sizeof(p1)/sizeof(p1[0]), DockLeft), getdocked(p2, sizeof(p2) / sizeof(p2[0]), DockLeftBottom));
	if(metrics::show::right)
		dock_paint(DockRight, rc, getdocked(p1, sizeof(p1) / sizeof(p1[0]), DockRight), getdocked(p2, sizeof(p2) / sizeof(p2[0]), DockRightBottom));
	if(metrics::show::bottom)
		dock_paint(DockBottom, rc, getdocked(p1, sizeof(p1) / sizeof(p1[0]), DockBottom), {});
}
}

static struct dockbar_settings_strategy : io::strategy {
	int getindex(const serializer::node& n, const char* name) const {
		if(!n.parent)
			return -1;
		auto text = n.parent->name;
		while(*name) {
			if(*name++ != *text++)
				return -1;
		}
		auto index = 0;
		text = stringbuilder::read(text, index);
		return index;
	}
	void write(serializer& file, void* param) override {
		for(auto i = DockLeft; i <= DockWorkspace; i = (dock_s)(i + 1)) {
			char temp[32]; stringbuilder sb(temp);
			sb.add("Dock%1i", i);
			file.open(temp);
			file.set("current", dock_data[i].current);
			file.set("size", dock_data[i].size);
			file.close(temp);
		}
	}
	void set(serializer::node& n, const char* value) override {
		unsigned i = getindex(n, "Dock");
		if(i == 0xFFFFFFFF || i >= sizeof(dock_data) / sizeof(dock_data[0]))
			return;
		if(n == "current")
			stringbuilder::read(value, dock_data[i].current);
		else if(n=="size")
			stringbuilder::read(value, dock_data[i].size);
	}
	dockbar_settings_strategy() : strategy("dockbar", "settings") {}
} dockbar_settings_strategy_instance;