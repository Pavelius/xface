#include "draw_control.h"
#include "fileimport.h"
#include "main.h"

static bool			show_grid;
static int			grid_size;
tileset*			current_tileset;

using namespace draw;

void logmsg(const char* format, ...);

struct tileset_control_type : controls::list, controls::control::plugin {
	control& getcontrol() override {
		return *this;
	}
	const char*	getlabel(char* result, const char* result_maximum) const override {
		return "Список тайлов";
	}
	static const char* getelementname(const void* p, char* result, const char* result_max) {
		return ((tileset*)p)->name;
	}
	void view(const rect& rc) override {
		rect r1 = rc;
		r1.y1 += field(r1.x1, r1.y1, r1.width(), "Набор", current, 50,
			bsdata<tileset>::source, getelementname, 0, 0, 0);
		list::view(r1);
	}
	tileset_control_type() : plugin("tile_list", DockRight) {
	}
};
static tileset_control_type tileset_control;

struct map_control_type : controls::scrollable, mapi, controls::control::plugin {

	control& getcontrol() override {
		return *this;
	}

	point m2s(const rect& rc, const point pt) const {
		point result;
		auto center = true;
		switch(type) {
		case IsometricRectangle:
			result.x = (pt.x / 2) * element.x + (pt.x % 2)*element.x / 2;
			result.y = pt.y * element.y / 2;
			break;
		default:
			result.x = pt.x * element.x;
			result.y = pt.y * element.y;
			break;
		}
		if(center) {
			result.x += element.x / 2;
			result.y += element.y / 2;
		}
		result.x += rc.x1 - offset.x;
		result.y += rc.y1 - offset.y;
		return result;
	}

	point getoffset(const rect& rc) const {
		point result = {};
		if(size.x == 0 || size.y == 0)
			return result;
		switch(type) {
		case Rectangle:
			result.x = rc.width() / element.x;
			result.y = rc.height() / element.y;
			break;
		}
		return result;
	}

	point getcamera() const {
		return {(short)origin.x, (short)origin.y};
	}

	void render_grid(const rect& rc) const {
		auto push_fore = fore;
		fore = colors::gray.mix(colors::window, 32);
		auto offset = getoffset(rc);
		point p1, p2;
		for(auto y = 0; y < size.y; y++) {
			p1 = m2s(rc, {(short)0, (short)y});
			p2 = m2s(rc, {(short)size.x, (short)y});
			p1.x -= element.x / 2;
			p1.y -= element.y / 2;
			p2.x -= element.x / 2;
			p2.y -= element.y / 2;
			line(p1.x, p1.y + element.y, p2.x, p2.y + element.y);
		}
		//line(p1.x, p1.y + element.y, p2.x, p2.y + element.y);
		for(auto x = 0; x < size.x; x++) {
			p1 = m2s(rc, {(short)x, (short)0});
			p2 = m2s(rc, {(short)x, (short)size.y});
			p1.x -= element.x / 2;
			p1.y -= element.y / 2;
			p2.x -= element.x / 2;
			p2.y -= element.y / 2;
			line(p1.x, p1.y, p2.x, p2.y);
		}
		//line(p1.x + element.x, p1.y, p2.x + element.x, p2.y);
		fore = push_fore;
	}

	void redraw(const rect& rc) override {
		render_grid(rc);
	}

	void change_type() {
		maximum.x = size.x * element.x;
		maximum.y = size.y * element.y;
	}

	map_control_type() : plugin("map", DockWorkspace) {
		type = IsometricRectangle;
		show_grid = true;
		element.x = 128;
		element.y = 64;
		size.x = 2;
		size.y = 4;
		change_type();
	}

};
static map_control_type map_control;

void tileset::import() {
}

tileimport::tileimport() {
	memset(this, 0, sizeof(*this));
}

static void testwizard() {
	struct tileset_wizard : wizard, tileimport, fileimport {
		bool choose_load_folder(const rect& rc, command_s id) {
			if(id == Draw) {
				auto x = rc.x1, y = rc.y1, w = rc.x2 - x;
				field(x, y, w, "Папка загрузки", source, sizeof(source), 150, choosefolder);
			}
			return true;
		}
		bool choose_sprite_name(const rect& rc, command_s id) {
			if(id == Draw) {
				auto x = rc.x1, y = rc.y1, w = rc.x2 - x;
				field(x, y, w, "Имя спрайта", destination, sizeof(destination), 150);
			}
			return true;
		}
		bool finish(const rect& rc, command_s id) {
			if(id == Initialize)
				execute();
			return true;
		}
		const element* getelements() const override {
			static element elements[] = {
				{"Укажите папку, в которой находятся все графические файлы, которые вы хотите импортировать в один файл спрайтов.", &tileset_wizard::choose_load_folder},
				{"Укажите имя итогового файла спрайта. Имя не должно содержать пробелов и не должно содержать никакое расширение. Система добавит расширение автоматически.", &tileset_wizard::choose_sprite_name},
				{"Спрайт успешно создан.", &tileset_wizard::finish}
			};
			return elements;
		}
		tileset_wizard() : fileimport("tileset", static_cast<tileimport*>(this), sizeof(tileimport)) {
		}
	};
	tileset_wizard wz;
	wz.show("Импорт тайлов");
}

void directory_initialize();

void run_main() {
	static shortcut keys[] = {{testwizard, "Тестирование мастера ввода", F1}};
	draw::application_initialize();
	directory_initialize();
	draw::application("X-Map editor", false, keys);
}