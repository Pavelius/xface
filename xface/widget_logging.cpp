#include "crt.h"
#include "datetime.h"
#include "draw_control.h"
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
INSTMETA(logi) = {
	BSREQ(stamp),
	BSREQ(text),
{}};

static arem<logi>	messages;

void logmsgv(const char* format, const char* arguments) {
	logi e = {0};
	char temp[8192]; stringbuilder sb(temp);
	sb.addv(format, arguments);
	e.stamp = datetime::now();
	e.text = szdup(temp);
	messages.add(e);
}

static void addsb(stringbuilder& sb, const datetime& d) {
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
		char temp[64]; stringbuilder sb(temp);
		addsb(sb, e.stamp);
		file << temp << " " << e.text << "\r\n";
	}
}

static void before_application_exit() {
	if(save_log_file)
		write_log_file();
}

static struct widget_logging : control::plugin, table {

	void after_initialize() override {
		auto meta = bsmeta<logi>::meta;
		addcol(meta, "stamp", "Дата").set(SizeFixed).set(AlignCenter);
		addcol(meta, "text", "Сообщение").set(SizeAuto);
	}

	control& getcontrol() override {
		return *this;
	}

	const char* getlabel(char* result, const char* result_maximum) const override {
		return "Список сообщений";
	}

	void* get(int line) const {
		return messages.data + line;
	}

	int	getmaximum() const override {
		return messages.getcount();
	}

	static void setting_common() {
		settings& e1 = settings::root.gr("Логирование").gr("Общие");
		e1.add("Сохранять файл сообщений после закрытия программы", save_log_file);
	}

	widget_logging() : control::plugin("logging", DockBottom) {
		no_change_count = true;
		read_only = true;
		select_mode = SelectRow;
		show_toolbar = false;
		setting_common();
		atexit(before_application_exit);
	}

} logging_control;

void logmsg(const char* format, ...) {
	logmsgv(format, xva_start(format));
}