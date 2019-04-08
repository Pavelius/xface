#include "xface/draw_control.h"
#include "main.h"

int main() {
	draw::application("Evrika", true);
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}