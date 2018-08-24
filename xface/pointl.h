#pragma once

struct pointl {
	int					x;
	int					y;
	inline bool			operator!=(const pointl pt) const { return pt.x != x || pt.y != y; }
	inline bool			operator==(const pointl pt) const { return pt.x == x && pt.y == y; }
};