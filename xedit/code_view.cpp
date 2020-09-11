#include "code_view.h"

using namespace draw;
using namespace draw::controls;

static bool isnum(const char* ps) {
	if(ps[0] >= '0' && ps[0] <= '9')
		return true;
	if(ps[0] == '-' && (ps[1] >= '0' && ps[1] <= '9'))
		return true;
	return false;
}