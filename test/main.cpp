#include "xface/bsreq.h"
#include "xface/collection.h"
#include "xface/crt.h"
#include "xface/draw_control.h"
#include "xface/variable.h"

using namespace draw;

struct testinfo {
	const char*		name;
	int				value;
};

struct radioreq : variable, runable {
	constexpr radioreq() : variable(), value() {}
	template<typename T> constexpr radioreq(T& v, unsigned value) : variable(v), value(value) {}
	void			execute() const override { current = *this; draw::execute(callback_proc); }
	int				getid() const { return (int)((char*)data + value); }
	unsigned		getfocus() const { return is() ? Checked : 0; }
	bool			is() const { return get() == value; }
private:
	static radioreq	current;
	static void callback_proc() {
		current.set(current.value);
	}
	unsigned		value;
};
radioreq radioreq::current;

struct checkreq : variable, runable {
	constexpr checkreq() : variable(), value() {}
	template<typename T> constexpr checkreq(T& v, unsigned value) : variable(v), value(value) {}
	void			execute() const override { current = *this; draw::execute(callback_proc); }
	int				getid() const { return (int)((char*)data + value); }
	unsigned		getfocus() const { return is() ? Checked : 0; }
	bool			is() const { return get() == value; }
private:
	static checkreq	current;
	static void callback_proc() {
		auto v = current.get() ^ current.value;
		current.set(v);
	}
	unsigned		value;
};
checkreq checkreq::current;

struct fieldreq : cmdfd {

	enum type_s { Text, Number };

	type_s			type;
	void*			source;
	unsigned		maximum;
	bool			isreference;
	controls::list* dropdown;

	static rect		current_rect;
	static fieldreq	current;

	fieldreq() = default;
	constexpr fieldreq(type_s type, void* source, unsigned size, bool isreference) : type(type), source(source), maximum(size), isreference(isreference), dropdown(0) {}
	constexpr fieldreq(aref<char> value) : fieldreq(Text, value.data, value.count - 1, false) {}
	constexpr fieldreq(const char* &value) : fieldreq(Text, &value, sizeof(value), true) {}
	constexpr fieldreq(int &value) : fieldreq(Number, &value, sizeof(value), false) {}
	constexpr fieldreq(short &value) : fieldreq(Number, &value, sizeof(value), false) {}
	constexpr fieldreq(char &value) : fieldreq(Number, &value, sizeof(value), false) {}

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

	static void callback_dropdown() {
	}

	int getid() const override {
		return (int)source;
	}

	bool choose(bool run) const override {
		if(!dropdown)
			return false;
		if(run) {
			current = *this;
			draw::execute(callback_dropdown);
		}
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
			case sizeof(char) : *((char*)source) = value; break;
				case sizeof(short) : *((short*)source) = value; break;
					case sizeof(int) : *((int*)source) = value; break;
					default: break;
		}
	}

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
		image(x, y, gres("monsters", "art/creatures"), 3, 0);
		image(x, y, gres("blood", "art/creatures"), tick % 3, 0);
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
		rc.y2 -= button(rc.x2 - 100, rc.y2 - draw::texth() - metrics::padding * 3, 100 - metrics::padding, 0, cmdx(buttoncancel), "Назад");
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

static void test_windget_button() {
	test_control();
}

static void test_widget() {
	struct element {
		const char*		name;
		int				mark;
		char			radio;
	};
	static bsreq element_type[] = {
		BSREQ(element, mark, number_type),
		BSREQ(element, radio, number_type),
	{}};
	static widget elements_left[] = {{Radio, "radio", "Samsung", 0},
	{Radio, "radio", "Nokia", 1},
	{Radio, "radio", "Google", 2},
	{}};
	static widget elements_right[] = {{Radio, "radio", "Noobie", 3},
	{Radio, "radio", "Glass", 4},
	{Radio, "radio", "Keeps", 5},
	{}};
	static widget brands[] = {{Group, 0, 0, 0, 4, 0, 0, elements_left},
	{Group, 0, 0, 0, 4, 0, 0, elements_right},
	{}};
	static widget elements[] = {{Check, "mark", "Простая пометка"},
	{Group, 0, "Выбирайте брэнд", 0, 0, 0, 0, brands},
	{Button, "button1", "Нажми, чтобы появлись элементы", 0, 0, 0, 0, 0, 0, test_windget_button},
	{}};
	element test = {0};
	while(ismodal()) {
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		draw::render(10, 10, 300, bsval(element_type, &test), elements);
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

static int radio(int x, int y, int width, unsigned& source, unsigned value, const char* title, const char* tips = 0) {
	radioreq cmd(source, value);
	auto result = draw::radio(x, y, width, cmd.getfocus(), cmd, title, tips);
	rect rc = {x, y, x + width, y + texth() + metrics::padding * 2};
	if(areb(rc)) {
		if(tips)
			statusbar("Радио кнопка %1", tips);
		else
			statusbar("Радио кнопка с координатами x=%1i, y=%2i", x, y);
	}
	return result;
}

static int checkbox(int x, int y, int width, unsigned& source, unsigned value, const char* title, const char* tips = 0) {
	checkreq cmd(source, value);
	auto result = draw::checkbox(x, y, width, cmd.getfocus(), cmd, title, tips);
	rect rc = {x, y, x + width, y + texth() + metrics::padding * 2};
	if(areb(rc)) {
		if(tips)
			statusbar("Чекбокс кнопка %1", tips);
		else
			statusbar("Чекбокс кнопка с координатами x=%1i, y=%2i", x, y);
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
	unsigned radio_button = 2;
	unsigned check_button = 0;
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		statusbardw();
		auto x = 10; auto y = 10;
		auto result = tabs(x, y, getwidth() - x * 2, false, false, (void**)elements, 0, sizeof(elements) / sizeof(elements[0]), 0, &current_hilite,
			get_text, {}); y += 40;
		if(current_hilite != -1)
			statusbar("Выбрать закладку '%1'", elements[current_hilite]);
		y += button(x, y, 200, button_accept, "Просто обычная кнопка", "Кнопка, которая отображает подсказку");
		y += button(x, y, 200, test_control, "Тестирование элементов");
		y += button(x, y, 200, test_widget, "Тестирование виджетов");
		//y += button(x, y, 200, test_edit, "Редактирование текста");
		y += button(x, y, 200, Disabled, cmdx(2), "Недоступная кнопка", "Кнопка, которая недоступная для нажатия");
		y += checkbox(x, y, 200, check_button, 2, "Галочка которая выводится и меняет значение чекбокса.");
		y += field(x, y, 400, t1, "Имя", 80, "Это подсказка текста для редактирования");
		y += field(x, y, 400, t2, "Количество", 80);
		y += radio(x, y, 200, radio_button, 1, "Первый элемент списка.");
		y += radio(x, y, 200, radio_button, 2, "Второй элемент возможного выбора.");
		y += radio(x, y, 200, radio_button, 3, "Третий элемент этого выбора.");
		auto id = input();
		switch(id) {
		case KeyEscape: return;
		default: dodialog(id); break;
		}
	}
}

static void test_array() {
	amem a1(sizeof(testinfo));
	auto p1 = (testinfo*)a1.add();
	p1->name = "Test";
	p1->value = 1;
	auto test_count = a1.getcount();
	p1 = (testinfo*)a1.add();
	p1->name = "New test";
	p1->value = 2;
	testinfo b1 = {"Third test", 3};
	p1 = (testinfo*)a1.insert(0, &b1);
	p1->name = "Third test";
	p1->value = 2;
}

void set_dark_theme();
void set_light_theme();

int main() {
	test_array();
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