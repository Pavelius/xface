#include "bsreq.h"
#include "crt.h"
#include "collection.h"
#include "datetime.h"
#include "draw_grid.h"
#include "settings.h"

using namespace	draw;
using namespace	draw::controls;

static bool save_log_file;

struct log_message {
	unsigned		stamp;
	const char*		text;
};
static arem<log_message> messages;
bsreq log_message_type[] = {
	BSREQ(log_message, stamp, number_type),
	BSREQ(log_message, text, text_type),
	{0}
};

void logmsgv(const char* format, const char* arguments) {
	log_message e = {0}; char temp[8192];
	szprintvs(temp, zendof(temp), format, arguments);
	e.stamp = datetime::now();
	e.text = szdup(temp);
	messages.add(e);
}

static struct widget_logging : control::plugin, grid {

	void after_initialize() override {
		addcol("date", "Дата", "datetime");
		addcol("text", "Сообщение", "text");
		setting_common();
	}

	control& getcontrol() override {
		return *this;
	}

	const char* getlabel(char* result, const char* result_maximum) const override {
		zcpy(result, "Список сообщений");
		return result;
	}

	static void setting_common() {
		settings& e1 = settings::root.gr("Логирование").gr("Общие");
		e1.add("Сохранять файл сообщений после закрытия программы", save_log_file);
	}

	widget_logging() : plugin("logging", DockBottom), grid(log_message_type, messages.data, sizeof(messages.data[0]), 0, messages.count) {
		no_change_count = true;
		read_only = true;
		select_mode = SelectRow;
	}

} logging_control;

void logmsg(const char* format, ...) {
	logmsgv(format, xva_start(format));
}