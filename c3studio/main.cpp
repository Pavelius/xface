#include "draw_control.h"
#include "lexer.h"
#include "package.h"

void add_package(const char* id);
void initialize_lexer();

void logmsg(const char* format, ...);

void draw::post(const char* id, const char* p1, unsigned p2) {
	if(equal(id, "editor.code.save")) {

	} else
		logmsg("Не обработано \"%1\" с параметрами %2, %3i", id, p1, p2);
}

int main() {
	initialize_lexer();
	code::urls::project = "code/projects/test/";
	code::urls::library = "code/library/";
	metrics::show::left = true;
	metrics::show::statusbar = true;
	add_package("main");
	add_package("core.crt");
	add_package("geo.rect");
	add_package("geo.point");
	draw::application("C3 studio", 0, 0, 0);
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}