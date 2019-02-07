#pragma once

struct bsval;
namespace draw {
enum control_s : unsigned char {
	NoField,
	Label, Field, Check, Radio, Button, Image,
	Tabs, Group,
	ControlMask = 0xF,
};
struct widget {
	unsigned			flags;
	const char*			id;
	const char*			label;
	int					value;
	int					width;
	int					height;
	int					title;
	const widget*		childs;
	const char*			tips;
	int					key;
	void(*proc)();
	operator bool() const { return flags != 0; }
};
int						render(int x, int y, int width, const bsval& value, const widget* element);
}