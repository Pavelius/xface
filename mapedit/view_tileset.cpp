#include "draw_control.h"
#include "io.h"
#include "main.h"

using namespace draw;

rect			sprite_get_border_rect(const sprite* ps);
static tileset*	current_tileset;

namespace {
class tilesetview : public controls::picker {
	const sprite* cashed;
	const char*	getlabel(stringbuilder& sb) const override {
		return "Набор тайлов";
	}
	int getmaximum() const override {
		auto ps = getsprite();
		if(!ps)
			return 0;
		return ps->count;
	}
	void row(const rect& rc, int index) override {
		auto ps = getsprite();
		if(!ps)
			return;
		draw::state push; setclip(rc);
		auto x = rc.x1 + rc.width() / 2;
		auto y = rc.y1 + rc.height() / 2;
		auto& fr = ps->get(index);
		x -= fr.sx / 2;
		y -= fr.sy / 2;
		image(x, y, ps, index, ImageNoOffset);
		rect r1 = {rc.x1, rc.y1, rc.x2 - 1, rc.y2 - 1};
		auto a = ishilite(r1);
		if(index == current) {
			rectf(r1, colors::edit, 96);
			if(isfocused())
				rectx(r1, colors::text.mix(colors::form, 200));
		}
	}
public:
	virtual const sprite* getsprite() const = 0;
	void view(const rect& rc) override {
		update();
		picker::view(rc);
	}
	void update() {
		auto ps = getsprite();
		if(ps == cashed)
			return;
		cashed = ps;
		if(ps) {
			pixels_per_line = cashed->height;
			pixels_per_column = cashed->width;
			auto rc = sprite_get_border_rect(cashed);
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
	tilesetview() :cashed(0) {
		show_grid_lines = false;
	}
	int getcurrentframe() {
		return current;
	}
};
struct control_type : public tilesetview, controls::control::plugin {
	control* getcontrol() override {
		return this;
	}
	const char*	getlabel(stringbuilder& sb) const override {
		return "Список тайлов";
	}
	void view(const rect& rc) override {
		rect r1 = rc;
		fnlist plist = {tileset::getname};
		r1.y1 += draw::field(r1.x1, r1.y1, r1.width(), "Набор", current_tileset, 50,
			bsdata<tileset>::source, 0, plist, 0);
		tilesetview::view(r1);
	}
	const sprite* getsprite() const override {
		if(!current_tileset)
			return 0;
		return current_tileset->getsprite();
	}
	control_type() : plugin("tile_list", DockRight) {
	}
};
}
static control_type object;

tileset* tileset::getcurrent() {
	return current_tileset;
}

int	tileset::getcurrentframe() {
	return object.getcurrentframe();
}

void tileset::setcurrent(tileset* v) {
	current_tileset = v;
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

struct spriteview : tilesetview {
	const sprite*				selected;
	const sprite* getsprite() const override {
		return selected;
	}
	void set(const sprite* v) {
		selected = v;
	}
};

const char* tileset::choosenew() {
	class view : public wizard {
		vector<spritei>			sprites;
		spriteview				preview;
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
			list.addcol("Наименование", ANREQ(spritei, name), "text");
			list.addcol("Спрайтов", ANREQ(spritei, count), "number");
			list.addcol("Размер", ANREQ(spritei, size), "number");
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
	if(!object.show("Добавление набора тайлов из библиотеки"))
		return 0;
	return object.getcurrent();
}