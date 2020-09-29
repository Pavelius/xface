#include "main.h"

void run_main();

int main() {
	current_tileset = tileset::add("monsters");
	run_main();
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}