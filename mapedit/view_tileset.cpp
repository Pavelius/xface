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
		if(v) {
			pixels_per_line = selected->height;
			pixels_per_column = selected->width;
			if(pixels_per_line > 128)
				pixels_per_line = 128;
			if(pixels_per_column > 196)
				pixels_per_column = 196;
			auto minimal = selected->getminsize();
			minimal.x += 2; minimal.y += 2;
			if(pixels_per_line <= minimal.y)
				pixels_per_line = minimal.y;
			if(pixels_per_column <= minimal.x)
				pixels_per_column = minimal.x;
		}
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

struct spritei {
	const char*		name;
	const sprite*	data;
	bool			error_sprite;
	const sprite* getsprite() {
		if(!data && !error_sprite) {
			char temp[260];
			data = (sprite*)loadb(tileset::geturl(temp, name));
			if(!data)
				error_sprite = true;
		}
		return data;
	}
	~spritei() {
		if(data)
			delete data;
	}
};
INSTMETA(spritei) = {BSREQ(name),
{}};

void add_tileset() {
	class view : public wizard {
		array					sprites_data;
		array::dataset<spritei>	sprites;
		tilesetview				preview;
		controls::tableview		list;
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
		const sprite* getcurrentsprite() const {
			auto p = (spritei*)list.getcurrent();
			if(!p)
				return 0;
			return p->getsprite();
		}
		bool mainpage(const rect& rc, command_s id) {
			if(id == Draw) {
				auto x1 = rc.x2 - 200;
				preview.set(getcurrentsprite());
				list.view({x1, rc.y1, rc.x2, rc.y2});
				preview.view({rc.x1, rc.y1, x1 - metrics::padding, rc.y2});
			}
			return true;
		}
		const element* getelements() const override {
			static element elements[] = {
				{"В правом окне выбирайте набор спрайтов. В левом окне работает предварительный просмотр.", &view::mainpage},
				{}};
			return elements;
		}
	public:
		view() : sprites_data(sizeof(spritei)), sprites(sprites_data), list(sprites_data) {
			list.show_header = false;
			list.addcol(bsmeta<spritei>::meta, "name", "Наименование");
			readsprites();
		}
	};
	view object;
	object.show("Добавение набора талов");
}