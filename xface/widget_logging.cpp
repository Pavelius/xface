#include "bsreq.h"
#include "crt.h"
#include "collection.h"
#include "datetime.h"
#include "draw_control.h"
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
}

void logmsg(const char* format, ...) {
	logmsgv(format, xva_start(format));
}

static struct widget_logging : table {

	void initialize() {
	}

	const char* getlabel(char* result, const char* result_maximum) const override {
		zcpy(result, "Список сообщений");
		return result;
	}

	static void setting_common() {
		settings& e1 = settings::root.gr("Логирование").gr("Общие");
		e1.add("Сохранять файл сообщений после закрытия программы", save_log_file);
	}

	widget_logging() {
		addcol("date", "Дата", "datetime");
		addcol("text", "Сообщение", "text");
		setting_common();
	}

} logging_control;
static control::plugin plugin("logging", logging_control, DockBottom);