#include "color.h"

#pragma once

namespace draw {
namespace controls {
struct control;
}
struct settings {
	enum types {
		Group,
		Bool, Int, Radio,
		Button, Color,
		TextPtr,
		UrlFolderPtr,
		Control,
	};
	types			type;
	const char*		name;
	settings*		next;
	int				value;
	void*			data;
	int				priority;
	void(*e_execute)();
	bool(*e_visible)(settings& e);
	//
	settings&		add(const char* name, bool& value);
	settings&		add(const char* name, int& value);
	settings&		add(const char* name, int& value, int current);
	settings&		add(const char* name, const char* &value, types type = TextPtr);
	settings&		add(const char* name, color& value);
	settings&		add(const char* name, draw::controls::control& value);
	settings&		add(const char* name, void(*fn)());
	settings*		child();
	settings&		execute(void(*value)());
	settings*		find(const char* identifier); // Find setting by identifier
	settings&		gr(const char* name, int priority = 0); // Add new group
	settings&		set(int value);
	settings&		set(bool(*fn)(settings& e));
	static settings	root;
};
}