#include "main.h"
#include "markup.h"

void run_main();

int main() {
	tileset::setcurrent(tileset::add("Monsters"));
	setproperties(bsdata<object>::elements, dginf<object>::meta);
	run_main();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}