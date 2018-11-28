#include "crt.h"
#include "draw_control.h"

using namespace draw::controls;

control::plugin::plugin(control& e) : element(e) {
	seqlink(this);
}