#include "draw_control.h"
#include "requisit.h"
#include "valuelist.h"
#include "main.h"

using namespace draw;

void logmsg(const char* format, ...);
void setproperties(void* object, const markup* type);

static unsigned radio_button = 2;
static unsigned check_button = 0;

static const char* product_category[] = {"Shoe", "T-Short", "Cap", "Book", "Phone", "Smartphone", "Pencil",
"Keyboard", "Mouse", "Headphones",
"Car", "Bus", "Flashmemory",
"Mute", "Guitar", "Piano",
"Soldier", "Heavy Soldier", "Sniper", "Commando"
};

BSDATA(genderi) = {{"NoGender", "����������"},
{"Male", "�������"},
{"Female", "�������"},
};
BSHEAD(genderi)
DGLNK(gender_s, genderi)
BSLNK(gender_s, genderi)

BSDATA(alignmenti) = {{"Neutral", "�����������"},
{"Lawful Good", "��������������� ������"},
{"Neutral Good", "���������� ������"},
{"Chaotic Good", "�������� ������"},
{"Lawful Neutral", "��������������� �����������"},
{"Chaotic Neutral", "�������� �����������"},
{"Lawful Evil", "��������������� ����"},
{"Neutral Evil", "���������� ����"},
{"Chaotic Evil", "�������� ����"},
};
BSHEAD(alignmenti)
DGLNK(alignment_s, alignmenti)
BSLNK(alignment_s, alignmenti)
BSDATAC(element, 32);
template<class T> const char* getnm(const void* object, stringbuilder& sb) {
	return ((T*)object)->name;
}
BSDATAC(rowelement, 64);
struct treerow : controls::tree::element {
	const char* name;
	int				value;
};
struct metatest {
	adat<char, 8>					source;
	std::initializer_list<char>		buffer;
};
struct testinfo {
	const char* name;
	int				value;
	int				value2;
	gender_s		gender;
};
struct markuptesti {
	const char* name;
	int				value;
	alignmenti* alignment;
	gender_s		gender;
	cflags<alignment_s> alignments;
};
DGINF(alignmenti) = {{"������������", DGREQ(name)}, {}};
DGINF(genderi) = {{"������������", DGREQ(name)}, {}};
DGINF(markuptesti) = {{"������������", DGREQ(name)},
{"��������", DGREQ(value)},
{"������������", DGREQ(alignment), {getnm<alignmenti>}},
{"���", DGREQ(gender), {getnm<genderi>}},
{"#chk ��������", DGREQ(alignments), {getnm<alignmenti>}},
{}};

static markuptesti test_properties_value = {"������", 12};

static int buttonx(int x, int y, int width, fnevent proc, const char* title, const char* tips = 0) {
	auto result = button(x, y, width, proc, title, tips);
	rect rc = {x, y, x + width, y + texth() + metrics::padding * 2};
	if(ishilite(rc)) {
		if(tips)
			statusbar("������: %1", tips);
		else
			statusbar("������ � ������������ x=%1i, y=%2i", x, y);
	}
	return result;
}

static int show_control(controls::control& e) {
	while(ismodal()) {
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		rc.offset(4 * 2);
		rc.y2 -= button(rc.x2 - 100 + metrics::padding, rc.y2 - draw::texth() - metrics::padding * 3, 100, buttoncancel, "�����", 0, KeyEscape);
		rc.y1 += e.toolbar(rc.x1, rc.y1, rc.width());
		e.view(rc);
		domodal();
	}
	return getresult();
}

static int show_table(controls::table& e) {
	setfocus(e, true);
	while(ismodal()) {
		rect rc = {0, 0, getwidth(), getheight()};
		rectf(rc, colors::form);
		rc.offset(4 * 2);
		button(rc.x2 - 100 + metrics::padding, rc.y2 - draw::texth() - metrics::padding - 2 * 4, 100, buttoncancel, "�����", 0, KeyEscape);
		auto y0 = rc.y2 - (draw::texth() + metrics::padding) * 3;
		auto x1 = rc.x1, y1 = y0, width = 300;
		y1 += checkbox(x1, y1, width, e.show_grid_lines, "��������� �����");
		y1 += checkbox(x1, y1, width, e.show_header, "��������� ���������");
		y1 += checkbox(x1, y1, width, e.hilite_odd_lines, "������������ �������� �����");
		x1 += width + metrics::padding, y1 = y0;
		y1 += checkbox(x1, y1, width, e.show_totals, "��������� �����");
		y1 += checkbox(x1, y1, width, e.show_border, "��������� �������");
		y1 += checkbox(x1, y1, width, e.show_background, "���������� ���");
		rc.y2 = y0 - metrics::padding;
		rc.y1 += e.toolbar(rc.x1, rc.y1, rc.width());
		e.view(rc);
		domodal();
	}
	return getresult();
}

static void basic_drawing() {
	while(ismodal()) {
		auto pf = fore;
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		triangle({360, 20}, {50, 300}, {500, 400}, colors::gray);
		rectf({100, 100, 300, 200}, colors::yellow, 196);
		rectb({100, 100, 300, 200}, colors::red);
		circlef(140, 140, 60, colors::red, 128);
		circle(140, 140, 60, colors::border);
		auto x = 100;
		auto y = 100;
		linw = 1;
		line(x, y, x + 100, y + 100); y += 40; linw = 2;
		line(x, y, x + 100, y + 100); y += 40; linw = 3;
		line(x, y, x + 100, y + 100); y += 40; linw = 4;
		line(x, y, x + 100, y + 100); y += 40; linw = 5;
		line(x, y, x + 60, y + 100);
		rectb({300, 100, 500, 300}, colors::green);
		linw = 1;
		button(10, 10, 100, buttoncancel, "������", 0, KeyEscape);
		domodal();
	}
}

static void test_tableref() {
	adat<rowelement, 32> elements;
	elements.add({"Pavel", Male, ChaoticEvil, 2, 30, 0, datetime::now()});
	elements.add({"Olga", Female, ChaoticGood, 0, 39, 0, datetime::now() - 5 * 24 * 60});
	elements.add({"Valentin", Male, NeutralGood, 1, 20, 0, datetime::now() - 3 * 24 * 60});
	elements.add({"Jorgun", Male, LawfulGood, 0, 16, 0, datetime::now() - 4 * 24 * 60});
	controls::tableref test;
	test.addcol("#", {}, "rownumber");
	test.addcol("�����������", ANREQ(rowelement, image), "image");
	test.addcol("������������", ANREQ(rowelement, name), "text");
	test.addcol("��", ANBIT(rowelement, flags, 1), "checkbox");
	test.addcol("��", ANBIT(rowelement, flags, 2), "checkbox");
	test.addcol("��", ANBIT(rowelement, flags, 3), "checkbox");
	test.addcol("�������", ANBIT(rowelement, age, 3), "number");
	//test.addcol(pm, "gender", "���").set(bsdata<genderi>::source_ptr);
	//test.addcol(pm, "alignment", "������������").set(bsdata<alignmenti>::source_ptr);
	//test.addcol(pm, "date", "����", "datetime");
	for(auto& e : elements)
		test.addref(&e);
	for(auto& e : elements)
		test.addref(&e);
	test.no_change_order = false;
	test.show_grid_lines = true;
	test.read_only = false;
	test.no_change_count = false;
	show_table(test);
}

static void test_tree() {
	struct test_tree_control : controls::tree {
		void expanding(int index, int level) override {
			treerow* p;
			p = (treerow*)insert(index, level);
			p->name = "Pavel";
			p->image = 1;
			p->value = 10;
			p->setgroup(true);
			p = (treerow*)insert(index, level);
			p->name = "Julia";
			p->image = 2;
			p = (treerow*)insert(index, level);
			p->name = "Peter";
			p->image = 4;
			p->value = 1000;
			p->setgroup(true);
		}
		constexpr test_tree_control() : tree(sizeof(treerow)) {}
	} test;
	test.select_mode = SelectText;
	test.addcol(0, ANREQ(treerow, image), "image");
	test.addcol("������������", ANREQ(treerow, name), "text").set(SizeAuto);
	test.addcol("��������", ANREQ(treerow, value), "number");
	test.expand(-1);
	show_table(test);
}

void apply_element(element* object) {
	object->age = 12;
}

static void test_list() {
	struct test_class : controls::list {
		const char* getname(stringbuilder& sb, int line, int column) const override {
			return product_category[line];
		}
		int getmaximum() const override {
			return sizeof(product_category) / sizeof(product_category[0]);
		}
	} test;
	show_control(test);
}

static const char* get_text(char* result, const char* result_maximum, void* object) {
	return (const char*)object;
}

static void disabled_button() {}

static void test_application() {
}

static void test_drag_drop() {
	auto x = 100, y = 200;
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		rect rc = {x, y, x + 50, y + 25};
		if(dragactive((void*)test_drag_drop)) {
			auto sx = rc.width();
			auto sy = rc.height();
			rc.x1 = x + (hot.mouse.x - dragmouse.x);
			rc.y1 = y + (hot.mouse.y - dragmouse.y);
			rc.x2 = rc.x1 + sx;
			rc.y2 = rc.y1 + sy;
			char temp[260]; stringbuilder sb(temp);
			sb.add("�������� ������ %1i, %2i", rc.x1, rc.y1);
			text(10, 42, sb);
		} else if(ishilite(rc) && hot.key == MouseLeft && hot.pressed)
			dragbegin((void*)test_drag_drop);
		if(dragactive((void*)test_drag_drop)) {
			line(rc.x1, rc.y1, dragmouse.x, dragmouse.y, colors::red);
			line(rc.x2, rc.y1, dragmouse.x, dragmouse.y, colors::blue);
			line(rc.x1, rc.y2, dragmouse.x, dragmouse.y, colors::green);
			line(rc.x2, rc.y2, dragmouse.x, dragmouse.y, colors::yellow);
		}
		rectf(rc, colors::form);
		rectb(rc, colors::border);
		button(10, 10, 100, buttoncancel, "������", 0, KeyEscape);
		domodal();
	}
}

static void choose_folder(const anyval& value) {
	char temp[260] = {};
	if(!dialog::folder("��������� �����", temp))
		return;
	value.set((int)szdup(temp));
}

static int point_input(int x, int y, point& result, int width, int title, const char* t1, const char* t2) {
	auto y0 = y;
	y += field(x, y, width, t1, result.x, title, 4);
	y += field(x, y, width, t2, result.y, title, 4);
	return y - y0;
}

static void choose_transparent_color() {
}

static int run_wizard(fnevent proc) {
	pushfocus pf;
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		auto x = 20, y = 20;
		y += button(x, y, 300, buttonok, "�������");
		domodal();
	}
}

static void test_autocomplite() {
	valuelist e;
	e.add("Kiew");
	e.add("Moskow");
	e.add("Lviv");
	e.add("Kherson");
	e.add("Mikolaev");
	auto p = e.choose(10, 10, 200, "M");
}

static void test_edit_field() {
	pushfocus pf;
	const char* name = "�����";
	const char* surname = "��������";
	const char* lastname = "�����������";
	const char* anystr = "����� ������";
	int number = 10;
	gender_s combo = Male;
	while(ismodal()) {
		auto x = 20, y = 20;
		rectf({0, 0, getwidth(), getheight()}, colors::form);
		auto h = draw::texth();
		auto w = 300;
		y += field(x, y, 300, "����", name, 100);
		y += field(x, y, 300, "��� ����", surname, 100);
		y += field(x, y, 300, "��� ����", lastname, 100);
		y += field(x, y, 300, "���� � �����", anystr, 100);
		y += field(x, y, 300, "���", combo, 100, bsdata<genderi>::source, controls::table::getenumname, 0);
		y += field(x, y, 300, "��������", number, 100, 4);
		y += button(x, y, 300, buttonok, "�������", "����� ��������� ������ ���������� ������");
		domodal();
	}
}

static void simple_controls() {
	pushfocus pf;
	int value = 10;
	auto v1 = false;
	auto v2 = true;
	while(ismodal()) {
		auto x = 20, y = 20;
		rectf({0, 0, getwidth(), getheight()}, colors::form);
		auto h = draw::texth();
		auto w = 300;
		y += radio(x, y, 300, {value, 10}, "������ ��������", "��� ����� ���������, ������� ���������� ��� ��������� �����");
		y += radio(x, y, 300, {value, 1}, "������ ��������", "������ ������� ����� ����� ���������");
		y += radio(x, y, 300, {value, 20}, "������ ��������");
		y += radio(x, y, 300, {value, 30}, "��������� ��������");
		y += checkbox(x, y, 300, v1, "������ �������") + metrics::padding;
		y += checkbox(x, y, 300, v2, "��� ���� �������, �� ��� � ����������, ������� �������� ��� ��������� �����. � ��� ������� ����� �������� �����.", "������ ��������� ��� �������.") + metrics::padding;
		y += button(x, y, 300, buttonok, "�������", "����� ��������� ������ ���������� ������");
		domodal();
	}
}

static void start_menu() {
	struct element {
		const char* name;
		fnevent			proc;
		const char* tips;
	};
	static element element_data[] = {{"����������� ���������", basic_drawing},
	{"��������������", test_drag_drop},
	{"������� ��������", simple_controls},
	{"������", test_list},
	{"������� � ��������", test_tableref},
	{"������", test_tree},
	{"���� �����", test_edit_field},
		//{"�����", test_tile_manager},
		//{"��������", test_markup},
	{"����������", draw::application},
	{"����������", test_autocomplite},
	{0}};
	while(ismodal()) {
		rectf({0, 0, getwidth(), getheight()}, colors::window);
		statusbardw();
		auto x = 10; auto y = 10;
		for(auto p = element_data; p->name; p++)
			y += buttonx(x, y, 200, p->proc, p->name, p->tips);
		domodal();
	}
}

static bool test_array() {
	array a1(sizeof(testinfo));
	auto p1 = (testinfo*)a1.add();
	memset(p1, 0, sizeof(testinfo));
	p1->name = "Test";
	p1->value = 1;
	auto test_count = a1.getcount();
	if(test_count != 1)
		return false;
	p1 = (testinfo*)a1.add();
	p1->name = "New test";
	p1->value = 2;
	test_count = a1.getcount();
	if(test_count != 2)
		return false;
	testinfo b1 = {"Third test", 3};
	auto p2 = (testinfo*)a1.insert(0, &b1);
	p2->name = "Third test";
	p2->value = 2;
	p2->value2 = 4;
	if(test_count != 2)
		return false;
	p1 = (testinfo*)a1.ptr(0);
	if(p1->value2 != 4)
		return false;
	a1.change((int)&((testinfo*)0)->value2, -(int)sizeof(testinfo::value2));
	p1 = (testinfo*)a1.ptr(0);
	p2 = (testinfo*)a1.ptr(1);
	a1.change((int)&((testinfo*)0)->value2, 12);
	p1 = (testinfo*)a1.ptr(0);
	p2 = (testinfo*)a1.ptr(1);
	return true;
}

static void test_requisit() {
	using namespace compiler;
	manager man;
	auto s1 = man.get("Test");
	auto s2 = man.get("Test");
	auto s3 = man.get("Test2");
	auto s4 = man.get("All correct");
	auto p2 = man.create("pointer");
	man.add(p2, "x", Number, 1, 2);
	man.add(p2, "y", Number, 1, 2);
	auto p1 = man.create("rect");
	man.add(p1, "x1", Number, 1, 4);
	man.add(p1, "y1", Number, 1, 4);
	man.add(p1, "x2", Number, 1, 4);
	man.add(p1, "y2", Number, 1, 4);
	auto p3 = man.create("region");
	man.add(p2, "rect", man.reference(p1));
	man.add(p2, "point", man.reference(p2), 2);
}

static bool test_datetime() {
	datetime d = datetime::now().daybegin() + 24 * 60;
	auto iy = d.year();
	auto im = d.month();
	auto id = d.day();
	return true;
}

static bool test_write_bin() {
	auto pa1 = bsdata<rowelement>::add();
	pa1->name = "Test";
	pa1->gender = Female;
	auto pa2 = bsdata<rowelement>::add();
	pa2->name = "Test2";
	pa2->gender = Male;
	element e1 = {0};
	element e2 = e1;
	// ��� ���������� ���������� ������������ ����� ���������.
	// ������� � ������ ������������ ���������� ���� ������� ����������� ��������������������� �������.
	// � ����� � ���� ���� ���������������� �������� �������� ���������.
	e1.refery[0] = pa1;
	e1.refery[3] = pa2;
	e1.refery[4] = pa1;
	e1.refery[6] = pa2;
	e1.age = 17;
	e1.name[0] = szdup("Kristian");
	e1.name[1] = szdup("��������");
	e1.surname = szdup("Lang");
	e1.gender = Male;
	e1.alignment = LawfulNeutral;
	//bsmeta<element>::meta->write("element.bin", &e1);
	//bsmeta<element>::meta->read("element.bin", &e2);
	//auto result = memcmp(&e1, &e2, sizeof(element));
	//return (result == 0);
	return true;
}

static bool test_data_access() {
	int result = 0;
	for(auto& e : bsdata<alignmenti>())
		result++;
	result = 0;
	for(auto& e : bsdata<genderi>())
		result++;
	result = 0;
	for(auto& e : bsdata<cultivatedi>())
		result++;
	return result != 0;
}

static bool test_anyval() {
	int a = 10;
	const anyval v2(a);
	const anyval v1(v2);
	return v1 == v2;
}

static bool test_initialize_list() {
	struct testlisti {
		const char* id;
		int			value;
		int			count;
		const std::initializer_list<int> elements;
	};
	static auto test_i35 = {3, 3, 3, 3, 3};
	static testlisti test_list_data[] = {
		{"Now", 3, 5, test_i35},
		{"Thing", 2, 3, {2, 2, 2}},
		{}
	};
	for(auto& e : test_list_data) {
		if(e.count != e.elements.size())
			return false;
		for(auto a : e.elements) {
			if(e.value != a)
				return false;
		}
	}
	return true;
}

static bool test_reestr() {
	reestr e;
	auto p1 = e.add(0xFFFFFFFF, "Root");
	auto p2 = e.add(p1, "Tables");
	auto p3 = e.add(p2, "New way");
	auto p4 = e.add(p3, "Columns");
	auto p5 = e.add(p4, "NewColumn");
	for(auto p : std::initializer_list<const char*>{"Name", "Id", "Text", "Value"})
		e.add(p5, p, 12);
	return true;
}

void directory_initialize();

int main() {
	if(!test_write_bin())
		return -1;
	if(!test_anyval())
		return -1;
	if(!test_array())
		return -1;
	if(!test_reestr())
		return -1;
	test_initialize_list();
	setproperties(&test_properties_value, dginf<markuptesti>::meta);
	directory_initialize();
	logmsg("Test %1i", 12);
	logmsg("Size of column %1i", sizeof(draw::controls::column));
	test_datetime();
	test_data_access();
	test_requisit();
	application_initialize();
	// �������� ����
	setcaption("X-Face C++ library samples");
	start_menu();
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}