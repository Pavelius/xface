#include "draw_control.h"
#include "lexer.h"
#include "package.h"

void add_package(const char* id);
void initialize_lexer();

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