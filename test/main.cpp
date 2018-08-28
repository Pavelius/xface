#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/draw_control.h"

using namespace draw;

class fieldreq : public cmdfd {

	enum type_s {Text, Number, DateTime, Date };

	type_s			type;
	void*			source;
	unsigned		maximum;
	bool			isreference;

	static rect		current_rect;
	static fieldreq	current;

	static void callback_edit() {
		auto focus = getfocus();
		char temp[260]; current.get(temp, temp + sizeof(temp) - 1, true);
		controls::textedit test(temp, sizeof(temp) - 1, true);
		setfocus((int)&test, true);
		test.editing(current_rect);
		current.set(temp);
		setfocus(focus, true);
	}

	static void callback_increment() {
		current.setnumber(current.getnumber() + hot.param);
	}

	int getid() const override {
		return (int)source;
	}

	bool choose(bool run) const override {
		return true;
	}

	bool increment(int step, bool run) const override {
		if(type != Number)
			return false;
		if(run) {
			current = *this;
			draw::execute(callback_increment, step);
		}
		return true;
	}

	void execute(const rect& rc) const override {
		current = *this;
		current_rect = rc;
		draw::execute(callback_edit);
	}

	int getnumber() const {
		switch(maximum) {
		case sizeof(char) : return *((char*)source);
		case sizeof(short) : return *((short*)source);
		case sizeof(int) : return *((int*)source);
		default: return 0;
		}
	}

	void setnumber(int value) const {
		switch(maximum) {
		case sizeof(char): *((char*)source) = value; break;
		case sizeof(short): *((short*)source) = value; break;
		case sizeof(int): *((int*)source) = value; break;
		default: break;
		}
	}

public:

	const char* get(char* result, const char* result_maximum, bool force_result) const {
		switch(type) {
		case Number:
			szprints(result, result_maximum, "%1i", getnumber());
			break;
		case Text:
			if(isreference) {
				auto p = *((const char**)source);
				if(!p)
					return "";
				if(!force_result)
					return p;
				szprints(result, result_maximum, p);
			} else {
				if(!force_result)
					return (const char*)source;
				unsigned maximum_count = result_maximum - result;
				if(maximum < maximum_count)
					maximum_count = maximum;
				zcpy(result, (const char*)source, maximum_count);
			}
			break;
		}
		return result;
	}

	void set(const char* result) const {
		switch(type) {
		case Text:
			if(!isreference)
				zcpy((char*)source, result, maximum);
			else {
				const char* p = 0;
				if(result[0])
					p = szdup(result);
				*((const char**)source) = p;
			}
			break;
		case Number:
			setnumber(sz2num(result));
			break;
		}
	}

	fieldreq() = default;
	constexpr fieldreq(aref<char> value) : type(Text), source(value.data), maximum(value.count - 1), isreference(false) {}
	constexpr fieldreq(const char* &value) : type(Text), source(&value), maximum(sizeof(value)), isreference(true) {}
	constexpr fieldreq(int &value) : type(Number), source(&value), maximum(sizeof(value)), isreference(false) {}
	constexpr fieldreq(short &value) : type(Number), source(&value), maximum(sizeof(value)), isreference(false) {}
	constexpr fieldreq(char &value) : type(Number), source(&value), maximum(sizeof(value)), isreference(false) {}

};

rect fieldreq::current_rect;
fieldreq fieldreq::current;

static void basic_drawing() {
	int tick = 10;
	settimer(100);
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		auto x = 100 + tick % 100;
		auto y = 100 + tick % 200;
		circlef(x, y, 50 + tick % 40, colors::form);
		circle(x, y, 50 + tick % 40, colors::border);
		auto id = input();
		switch(id) {
		case InputTimer:
			tick++;
			break;
		case KeyEscape:
			buttoncancel();
			break;
		default: dodialog(id); break;
		}
	}
	settimer(0);
}

static void button_accept() {
	basic_drawing();
}

static void test_control() {
	static controls::column columns[] = {{Text, "name", "Наименование", 200},
	{Number, "count", "К-во", 32},
	{}};
	controls::table test(columns);
	while(ismodal()) {
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		rc.offset(4 * 2);
		rc.y2 -= button(rc.x2 - 100, rc.y2 - draw::texth() - metrics::padding * 2, 100 - metrics::padding, 0, cmdx(buttoncancel), "Назад");
		test.view(rc);
		auto id = input();
		switch(id) {
		case KeyEscape:
			buttoncancel();
			break;
		default: dodialog(id); break;
		}
	}
}

static const char* get_text(char* result, const char* result_maximum, void* object) {
	return (const char*)object;
}

static int button(int x, int y, int width, void(*proc)(), const char* title, const char* tips = 0) {
	auto result = button(x, y, width, 0, cmdx(proc), title, tips);
	rect rc = {x, y, x + width, y + texth() + metrics::padding * 2};
	if(areb(rc)) {
		if(tips)
			statusbar("Кнопка: %1", tips);
		else
			statusbar("Кнопка с координатами x=%1i, y=%2i", x, y);
	}
	return result;
}

template<typename T> static int field(int x, int y, int width, T& value, const char* title, int title_width, const char* tips = 0) {
	char temp[260];
	fieldreq field_requisit(value);
	return field(x, y, width, TextSingleLine, field_requisit, field_requisit.get(temp, temp + sizeof(temp) - 1, false), tips, title, title_width);
}

static void simple_controls() {
	static const char* elements[] = {"Файл", "Правка", "Вид", "Окна"};
	setfocus(3, true);
	int current_hilite;
	const char* t1 = "Тест 1";
	char t2 = 20;
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		statusbardw();
		auto x = 10; auto y = 10;
		auto result = tabs(x, y, getwidth() - x*2, false, false, (void**)elements, 0, sizeof(elements) / sizeof(elements[0]), 0, &current_hilite,
			get_text, {}); y += 40;
		if(current_hilite != -1)
			statusbar("Выбрать закладку '%1'", elements[current_hilite]);
		y += button(x, y, 200, button_accept, "Просто обычная кнопка", "Кнопка, которая отображает подсказку");
		y += button(x, y, 200, test_control, "Тестирование элементов");
		//y += button(x, y, 200, test_edit, "Редактирование текста");
		y += button(x, y, 200, Disabled, cmdx(2), "Недоступная кнопка", "Кнопка, которая недоступная для нажатия");
		y += checkbox(x, y, 200, 0, cmdx(3), "Галочка которая выводится и меняет значение чекбокса.");
		y += field(x, y, 400, t1, "Имя", 80, "Это подсказка текста для редактирования");
		y += field(x, y, 400, t2, "Количество", 80);
		y += radio(x, y, 200, 0, cmdx(4), "Первый элемент списка.");
		y += radio(x, y, 200, 0, cmdx(5), "Второй элемент возможного выбора.");
		y += radio(x, y, 200, Checked, cmdx(6), "Третий элемент этого выбора.");
		auto id = input();
		switch(id) {
		case KeyEscape: return;
		default: dodialog(id); break;
		}
	}
}

void set_dark_theme();
void set_light_theme();

int main() {
	set_light_theme();
	// Инициализация библиотеки
	initialize();
	// Создание окна
	create(-1, -1, 640, 480, WFResize | WFMinmax, 32);
	setcaption("X-Face C++ library");
	simple_controls();
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}