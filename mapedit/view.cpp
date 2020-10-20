#include "draw_control.h"
#include "fileimport.h"
#include "main.h"

static bool			show_grid;
static int			grid_size;
static point		current_mouse;

using namespace draw;

object*	object::add(point pt) {
	auto p = bsdata<object>::add();
	p->kind = current_tileset;
	p->frame = current_tileset->getcurrentframe();
	p->x = pt.x;
	p->y = pt.y;
	return p;
}

void object::draw(point camera) const {
	auto ts = kind.getresource();
	if(!ts)
		return;
	auto sp = ts->getsprite();
	if(!sp)
		return;
	draw::image(x, y, sp, frame, flags);
}

struct map_control_type : controls::scrollable, mapi, controls::control::plugin {
	control* getcontrol() override {
		return this;
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
	static void add_object() {
		object::add(current_mouse);
	}
	void redraw(const rect& rc) override {
		//render_grid(rc);
		point screen;
		screen.x = rc.x1 - origin.x;
		screen.y = rc.y1 - origin.y;
		for(auto& e : bsdata<object>())
			e.draw(screen);
		if(ishilite(rc)) {
			current_mouse.x = screen.x + hot.mouse.x - rc.x1;
			current_mouse.y = screen.y + hot.mouse.y - rc.y1;
			switch(hot.key) {
			case MouseLeft:
				if(hot.pressed)
					execute(add_object);
				break;
			}
		}
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
		strlib			strings;
		struct file {
			unsigned	id;
		};
		vector<file>	files;
		static const char* getdirectionname(const void* object, stringbuilder& sb) {
			return ((directioni*)object)->name;
		}
		static bool horizallow(const void* object, int index) {
			switch(index) {
			case Left: case Right: case Center: return true;
			default: return false;
			}
		}
		static bool vertallow(const void* object, int index) {
			switch(index) {
			case Up: case Down: case Center: return true;
			default: return false;
			}
		}
		bool choose_load_folder(const rect& rc, command_s id) {
			if(id == Draw) {
				auto x = rc.x1, y = rc.y1, w = rc.x2 - x;
				field(x, y, w, "Папка загрузки", source, sizeof(source), 150, 0/*choosefolder*/);
			}
			return true;
		}
		bool centroid(const rect& rc, command_s id) {
			if(id == Draw) {
				auto x = rc.x1, y = rc.y1, w = rc.x2 - x;
				auto w1 = imin(w/2, 350);
				auto w2 = imin(w/2, 350);
				field(x, y, w1, "Смещение по горизонтале", base_x, 180, bsdata<directioni>::source, 0, {getdirectionname, horizallow});
				y += field(x + w1 + metrics::padding, y, w1, "на значение", offset.x, 100, 4);
				field(x, y, w1, "Смещение по вертикале", base_y, 180, bsdata<directioni>::source, 0, {getdirectionname, vertallow});
				y += field(x + w1 + metrics::padding, y, w1, "на значение", offset.y, 100, 4);
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
				{"В каждом спрайте будет находится центральная точка, которая будет использовать для вывода спрайта на экран. Укажите смещение этой точки относительно прямоугольника, описывающим спрайт.", &tileset_wizard::centroid},
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
void update_tileset();

static void heartbreak() {
	update_tileset();
}

void run_main() {
	static shortcut keys[] = {{testwizard, "Тестирование мастера ввода", Ctrl + Alpha + '1'},
	{add_tileset, "Тестирование добавление спрайтов", Ctrl + Alpha + '2'},
	{}};
	logmsg("Старт системы");
	draw::application("X-Map editor",0, heartbreak, keys);
}