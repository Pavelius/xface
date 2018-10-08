#include "rect.h"

#pragma once

struct point {
	short int			x, y;
	explicit			operator bool() const { return x || y; }
	bool				operator!=(const point pt) const { return pt.x != x || pt.y != y; }
	bool				operator==(const point pt) const { return pt.x == x && pt.y == y; }
	point				operator-(const point pt) const { return{x - pt.x, y - pt.y}; }
	point				operator+(const point pt) const { return{x + pt.x, y + pt.y}; }
	void				clear() { x = y = 0; }
	bool				in(const rect& rc) const { return x >= rc.x1 && x <= rc.x2 && y >= rc.y1 && y <= rc.y2; }
	bool				in(const point p1, const point p2, const point p3) const;
	static point		create(int n) { return {(short)((unsigned)n & 0xFFFF), (short)(((unsigned)n) >> 16)}; }
	void				set(int px, int py) { x = (short)px; y = (short)py; }
};