#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

struct toolbar_builder : control::command::builder {

	int		x, y, x2;
	point	size;
	constexpr toolbar_builder(int x, int y, int width, const point size) : x(x), y(y), x2(x + width), size(size) {}
	void addseparator() override {
		gradv({x + 3, y + 4, x + 5, y + size.y - 4}, colors::border.lighten(), colors::border);
		x += 7;
	}
	void add(const control* source, const control::command& cmd) override {
		auto width = 0;
		if(cmd.view == ViewIcon || cmd.view == ViewIconAndText)
			width += size.x;
		if(cmd.view == ViewIconAndText || cmd.view == ViewText) {
			auto w = draw::textw(cmd.name);
			width += metrics::padding * 2 + w;
		}
		rect rc = {x, y, x + width, y + size.y};
		if(ishilite(rc)) {
			auto name = cmd.name;
			if(name) {
				if(cmd.key) {
					char temp[128];
					tooltips("%1 (%2)", name, key2str(temp, cmd.key));
				} else
					tooltips(name);
			}
			statusbar("��������� ������� '%1'", name);
		}
		auto disabled = !cmd.isallow(source);
		if(draw::tool(rc, disabled, false, true))
			source->postcmd(cmd.proc.cmd);
		if(cmd.view == ViewIcon || cmd.view == ViewIconAndText)
			source->icon(x + size.x / 2, y + size.y / 2, disabled, cmd);
		if(cmd.view == ViewText || cmd.view == ViewIconAndText)
			draw::textc(x, y + (size.y - draw::texth()) / 2, rc.x2 - x, cmd.name);
		x += width;
	}
};

int	draw::controls::control::toolbar(int x, int y, int width, int* next_x) const {
	auto commands = getcommands();
	if(next_x)
		*next_x = x;
	if(!commands)
		return 0;
	auto images = getimages();
	if(!images)
		return 0;
	short height = images->get(0).getrect(0, 0, 0).height() + 4;
	toolbar_builder e(x, y, width, {height, height});
	e.render(this, getcommands());
	if(next_x)
		*next_x = e.x;
	if(e.x != x)
		return height + metrics::padding;
	return 0;
}