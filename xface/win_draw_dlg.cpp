#include "stringbuilder.h"
#include "win.h"

void dlgmsg(const char* title, const char* text) {
	MessageBoxA(GetActiveWindow(), text, title, 0);
}

void dlginf(const char* title, const char* text) {
	MessageBoxA(GetActiveWindow(), text, title, MB_OK | MB_ICONINFORMATION);
}

void dlgerr(const char* title, const char* format, ...) {
	char temp[4096]; stringbuilder sc(temp);
	sc.addv(format, xva_start(format));
	MessageBoxA(GetActiveWindow(), temp, title, MB_OK | MB_ICONERROR);
}

bool dlgask(const char* title, const char* text) {
	return (MessageBoxA(GetActiveWindow(), text, title, MB_YESNO | MB_ICONQUESTION)) == IDYES;
}