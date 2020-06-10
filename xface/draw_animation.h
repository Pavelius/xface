#include "point.h"

#pragma once

struct sprite;

namespace draw {
struct animation {
	sprite*				res;
	unsigned short		frame, start, stop, speed;
	point				pos;
	unsigned char		alpha;
	char				height;
	unsigned			flags;
	unsigned			data;
	unsigned			timestamp;
	void				paint(int x, int y) const;
	void				update();
};
class stage {
	constexpr static unsigned count_maximum = 512;
	animation			source[count_maximum];
	unsigned			count;
public:
	stage() : count(0) {}
	animation*			add();
	animation*			add(unsigned data);
	animation*			begin() { return source; }
	const animation*	begin() const { return source; }
	void				clear() { count = 0; }
	animation*			end() { return source + count; }
	const animation*	end() const { return source + count; }
	animation*			find(unsigned data);
	void				paint(point camera) const;
	animation*			test(point mouse);
	void				update(unsigned timestamp);
};
}