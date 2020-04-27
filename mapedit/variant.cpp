#include "main.h"

resourcei* variant::getresource() const {
	if(!type)
		return 0;
	return bsdata<resourcei>::elements + value;
}