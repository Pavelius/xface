#include "draw.h"
#include "draw_animation.h"

using namespace draw;

void animation::paint(int x, int y) const {
	image(x + pos.x, y + pos.y, res, frame, flags, alpha);
}

animation* stage::add() {
	if(count >= count_maximum)
		return source;
	return source + (count++);
}