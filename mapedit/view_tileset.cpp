#include "draw_control.h"
#include "io.h"
#include "main.h"

using namespace draw;

namespace {
class tilesetview : public controls::picker {
	const sprite* selected;
	const char*	getlabel(char* result, const char* result_maximum) const override {
		return "Набор тайлов";
	}
	int getmaximum() const override {
		if(!selected)
			return 0;
		return selected->count;
	}
	void row(const rect& rc, int index) override {
		draw::state push; setclip(rc);
		auto x = rc.x1 + rc.width() / 2;
		auto y = rc.y1 + rc.height() / 2;
		image(x, y, selected, index, 0);
		rect r1 = {rc.x1, rc.y1, rc.x2 - 1, rc.y2 - 1};
		auto a = ishilite(r1);
		if(index == current) {
			rectf(r1, colors::edit, 96);
			if(isfocused())
				rectx(r1, colors::text.mix(colors::form, 200));
		}
	}
public:
	void set(const sprite* v) {
		if(selected == v)
			return;
		selected = v;
		pixels_per_line = selected->height;
		pixels_per_column = selected->width;
		if(pixels_per_line > 128)
			pixels_per_line = 128;
		if(pixels_per_column > 196)
			pixels_per_column = 196;
	}
	tilesetview() :selected(0) {
		show_grid_lines = false;
	}
};
struct control_type : public tilesetview, controls::control::plugin {
	tileset* selected;
	control& getcontrol() override {
		return *this;
	}
	const char*	getlabel(char* result, const char* result_maximum) const override {
		return "Список тайлов";
	}
	void view(const rect& rc) override {
		rect r1 = rc;
		r1.y1 += field(r1.x1, r1.y1, r1.width(), "Набор", current_tileset, 50,
			bsdata<tileset>::source, tileset::getname, 0, 0, 0);
		tilesetview::view(r1);
	}
	void set(tileset* v) {
		if(selected == v)
			return;
		selected = v;
		tilesetview::set(selected->getsprite());
	}
	control_type() : plugin("tile_list", DockRight), selected(0) {
	}
};
}
static control_type object;

void update_tileset() {
	object.set(current_tileset);
}

void add_tileset() {
	struct spritei {
		const char*		name;
	};
	struct view : wizard {
		arem<spritei>		sprites;
		tilesetview			main;
		controls::table		list;
		void readsprites() {
			sprites.clear();
			for(io::file::find f(tileset::base_url); f; f.next()) {
				auto pn = f.name();
				if(pn[0] == '.')
					continue;
				auto p = sprites.add();
				memset(p, 0, sizeof(*p));
				char temp[260]; szfnamewe(temp, f.name());
				p->name = szdup(temp);
			}
		}
		bool mainpage(const rect& rc, command_s id) {
			if(id == Draw) {
				auto x1 = rc.x2 - 200;
				list.view({x1, rc.y1, rc.x2, rc.y2});
				main.view({rc.x1, rc.y1, x1 - metrics::padding, rc.y2});
			}
			return true;
		}
		const element* getelements() const override {
			static element elements[] = {
				{"В правом окне выбирайте набор спрайтов. В левом окне работает предварительный просмотр.", &view::mainpage},
				{}};
			return elements;
		}
		view() {
			list.show_header = false;
			readsprites();
		}
	};
	view object;
	object.show("Добавение набора талов");
}