#include "main.h"

wound_s combati::shoot(char bonus) const {
	auto i = rand() % 100;
	auto n = miss + bonus;
	if(i < n)
		return NoWound;
	n += light;
	if(i < n)
		return LightWound;
	n += heavy;
	if(i < n)
		return HeavyWound;
	if(n > 100)
		return NoWound;
	return FatalWound;
}

wound_s combati::suffer(wound_s w) const {
	auto i = rand() % 100;
	switch(w) {
	case LightWound:
		if(i < light)
			return NoWound;
		break;
	case HeavyWound:
		if(i < heavy)
			return LightWound;
		break;
	case FatalWound:
		if(i < fatal)
			return HeavyWound;
		break;
	}
	return w;
}