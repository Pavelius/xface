#include "main.h"
#include "draw_control.h"

using namespace code;
using namespace draw;

class widget_reqtable : public controls::table {
	const requisit*		source;
	array				source_array;
	widget_reqtable(const requisit* v) : source(v), source_array(v->getlenght()) {
		auto parent_type = v->type;
		for(auto& e : bsdata<requisit>()) {
		}
	}
};