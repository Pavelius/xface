#pragma once

struct rect {
	int					x1, y1;
	int					x2, y2;
	//
	operator bool() const { return x1 != 0 || x2 != 0 || y1 != 0 || y2 != 0; }
	inline bool			operator!=(const rect& rc) const { return rc.x1 != x1 || rc.x2 != x2 || rc.y1 != y1 || rc.y2 != y2; }
	inline bool			operator==(const rect& rc) const { return rc.x1 == x1 && rc.x2 == x2 && rc.y1 == y1 && rc.y2 == y2; }
	inline rect			operator+(const rect& rc) const { return{x1 + rc.x1, y1 + rc.y1, x2 + rc.x2, y2 + rc.y2}; }
	inline rect			operator+(const int v) const { return{x1 + v, y1 + v, x2 - v, y2 - v}; }
	inline rect			operator-(const int v) const { return{x1 - v, y1 - v, x2 + v, y2 + v}; }
	void				clear() { x1 = x2 = y1 = y2 = 0; }
	inline int			height() const { return y2 - y1; }
	bool				intersect(const rect& rc) const { return x1<rc.x2 && x2>rc.x1 && y1<rc.y2 && y2>rc.y1; }
	void				move(int x, int y) { x1 += x; x2 += x; y1 += y; y2 += y; }
	void				offset(int x, int y) { x1 += x; x2 -= x; y1 += y; y2 -= y; }
	void				offset(int n) { offset(n, n); }
	void				set(int px1, int py1, int px2, int py2) { x1 = px1; x2 = px2; y1 = py1; y2 = py2; }
	inline int			width() const { return x2 - x1; }
};