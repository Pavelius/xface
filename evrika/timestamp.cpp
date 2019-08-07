#include "main.h"

void stampi::write() {
	if(!isnew())
		return;
	counter &= 0xFF000000;
	counter |= getbase().getcount();
}