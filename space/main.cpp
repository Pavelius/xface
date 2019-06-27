#include "main.h"

static combati light_weapon = {40, 60};
static combati heavy_weapon = {10, 20, 20, 50};
static combati light_armor = {10, 30};

int main() {
	uniti u1 = Soldiers;
	wound_s w1[16];
	wound_s w2[16];
	for(auto& e : w1)
		e = heavy_weapon.shoot(light_armor.miss);
	for(unsigned i = 0; i<sizeof(w2)/ sizeof(w2[0]); i++)
		w2[i] = light_armor.suffer(w1[i]);
	u1.apply(light_weapon.shoot(light_armor.miss));
	u1.apply(light_weapon.shoot(light_armor.miss));
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main();
}