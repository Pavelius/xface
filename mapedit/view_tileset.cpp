#include "draw_control.h"
#include "io.h"
#include "main.h"

using namespace draw;

rect		sprite_get_border_rect(const sprite* ps);
tileset*	current_tileset;

namespace {
class tilesetview : public controls::picker {
	const sprite* selected;
	const char*	getlabel(stringbuilder& sb) const override {
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
		auto& fr = selected->get(index);
		x -= fr.sx / 2;
		y -= fr.sy / 2;
		image(x, y, selected, index, ImageNoOffset);
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
			auto rc = sprite_get_border_rect(selected);
			rc.offset(-2);
			if(pixels_per_line <= rc.height())
				pixels_per_line = rc.height();
			if(pixels_per_column <= rc.width())
				pixels_per_column = rc.width();
			if(pixels_per_line > 128)
				pixels_per_line = 128;
			if(pixels_per_column > 196)
				pixels_per_column = 196;
		}
	}
	tilesetview() :selected(0) {
		show_grid_lines = false;
	}
	int getcurrent() {
		return current;
	}
};
struct control_type : public tilesetview, controls::control::plugin {
	tileset* selected;
	control* getcontrol() override {
		return this;
	}
	const char*	getlabel(stringbuilder& sb) const override {
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

int	tileset::getcurrentframe() {
	return object.getcurrent();
}

void update_tileset() {
	object.set(current_tileset);
}

struct spritei : sprite {
	const char*		name;
	const sprite*	data;
	bool			error_sprite;
	const char* getfilename(char* buffer) const {
		return tileset::geturl(buffer, name);
	}
	const sprite* getsprite() {
		if(!data && !error_sprite) {
			char temp[260];
			data = (sprite*)loadb(getfilename(temp));
			if(!data)
				error_sprite = true;
		}
		return data;
	}
	void readheader() {
		char temp[260];
		io::file file(getfilename(temp), StreamRead);
		if(!file)
			return;
		file.read(static_cast<sprite*>(this), sizeof(sprite));
	}
	~spritei() {
		if(data)
			delete data;
	}
};
BSMETA(spritei) = {BSREQ(name), BSREQ(size), BSREQ(count),
{}};

const char* tileset::choosenew() {
	class view : public wizard {
		vector<spritei>			sprites;
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
				p->readheader();
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
				auto x2 = rc.x1 + 400;
				list.view({rc.x1, rc.y1, x2 - metrics::padding, rc.y2});
				preview.set(getcurrentsprite());
				preview.view({x2, rc.y1, rc.x2, rc.y2});
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
		view() : list(sprites) {
			list.read_only = true;
			list.select_mode = SelectRow;
			//list.addcol(bsmeta<spritei>::meta, "name", "Наименование");
			//list.addcol(bsmeta<spritei>::meta, "count", "Спрайтов");
			//list.addcol(bsmeta<spritei>::meta, "size", "Размер");
			readsprites();
		}
		const char* getcurrent() const {
			auto p = (spritei*)list.getcurrent();
			if(p)
				return p->name;
			return 0;
		}
	};
	view object;
	if(!object.show("Добавение набора тайлов"))
		return 0;
	return object.getcurrent();
}