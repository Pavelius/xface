#include "draw_control.h"
#include "io.h"
#include "lexer.h"
#include "package.h"

void add_package(const char* id);
void update_package(code::package* p);
void initialize_lexer();

void logmsg(const char* format, ...);

static void get_url_ast(stringbuilder& sb, const char*) {
}

static void get_current_dir() {
	char temp[260];
	io::file::getdir(temp, sizeof(temp));
	szlower(temp);
	szchange(temp, '\\', '/');
}

void draw::post(const char* id, const char* p1, unsigned p2) {
	if(equal(id, "editor.code.save")) {
		//update_package((code::package*)p2);
	} else
		logmsg("Не обработано \"%1\" с параметрами %2, %3i", id, p1, p2);
}

int main() {
	initialize_lexer();
	code::urls::project = "code/projects/test/";
	code::urls::library = "code/library/";
	metrics::show::left = true;
	metrics::show::statusbar = true;
	get_current_dir();
	add_package("main");
	add_package("core.crt");
	add_package("geo.rect");
	add_package("geo.point");
	draw::application("C3 studio", 0, 0, 0);
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}