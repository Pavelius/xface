#include "datetime.h"
#include "win.h"

datetime datetime::now() {
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	return datetime(tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute);
}
