#include "crt.h"
#include "datetime.h"
#include "draw_grid.h"
#include "io.h"
#include "settings.h"
#include "stringbuilder.h"

using namespace	draw;
using namespace	draw::controls;

static bool save_log_file;

namespace {
struct logi {
	datetime		stamp;
	const char*		text;
};
}
const bsreq bsmeta<logi>::meta[] = {
	BSREQ(stamp),
	BSREQ(text),
{}};

static arem<logi> messages;

void logmsgv(const char* format, const char* arguments) {
	logi e = {0};
	char temp[8192]; stringbuilder sb(temp);
	sb.addv(format, arguments);
	e.stamp = datetime::now();
	e.text = szdup(temp);
	messages.add(e);
}

static void add_date(char* result, const char* result_max, const datetime& d) {
	stringbuilder sb(result, result_max);
	sb.add("%1.2i.%2.2i.%3.2i %4.2i:%5.2i",
		d.day(), d.month(), d.year(),
		d.hour(), d.minute());
}

static void write_log_file() {
	io::file file("log.txt", StreamRead | StreamWrite | StreamText);
	if(!file)
		return;
	file.seek(0, SeekEnd);
	for(auto& e : messages) {
		char temp[64]; add_date(temp, zendof(temp), e.stamp);
		file << temp << " " << e.text << "\r\n";
	}
}

static void before_application_exit() {
	if(save_log_file)
		write_log_file();
}

static struct widget_logging : control::plugin, table {

	void after_initialize() override {
		addcol(bsmeta<logi>::meta, "stamp", "Дата").setwidth(textw("0") * 16 + 4).set(SizeFixed).set(AlignCenter);
		addcol(bsmeta<logi>::meta, "text", "Сообщение").set(SizeAuto);
	}

	control& getcontrol() override {
		return *this;
	}

	const char* getlabel(char* result, const char* result_maximum) const override {
		return "Список сообщений";
	}

	static void setting_common() {
		settings& e1 = settings::root.gr("Логирование").gr("Общие");
		e1.add("Сохранять файл сообщений после закрытия программы", save_log_file);
	}

	void* get(int line) const {
		return messages.data + line;
	}
		
	int	getmaximum() const override {
		return messages.getcount();
	}

	widget_logging() : plugin("logging", DockBottom) {
		no_change_count = true;
		read_only = true;
		select_mode = SelectRow;
		setting_common();
		atexit(before_application_exit);
	}

} logging_control;

void logmsg(const char* format, ...) {
	logmsgv(format, xva_start(format));
}