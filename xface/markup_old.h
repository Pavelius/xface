#pragma once

typedef bool(*fnallow)(const void* object, int index); // Is allow some property
typedef void(*fnchange)(void* object, const void* previous_object); // Change object
typedef void(*fncommand)(void* object); // Object's actions
typedef int(*fndraw)(int x, int y, int width, const void* object); // Custom draw
typedef const char* (*fntext)(const void* object, char* result, const char* result_maximum);

// Standart markup
struct markup {
	struct element {
		const char*		id;			// Field identificator (0 for group)
		int				index;		// Array index
		const markup*	child;		// Group or next field
	};
	struct cmdi {
		fncommand		execute;
		fnchange		change;
		constexpr cmdi() : execute(0), change(0) {}
		template<class T> constexpr cmdi(void(*v)(T*)) : execute((fncommand)v), change(0) {}
		template<class T> constexpr cmdi(void(*v)(T*, const T*)) : execute(0), change((fnchange)v) {}
	};
	struct proci {
		fnallow			isallow;	// Is allow special element or command
		fnallow			isvisible;	// Is element visible
		fndraw			custom;
	};
	struct propi {
		fntext			getname;
		constexpr propi() : getname(0) {}
		template<class T> constexpr propi(void(*v)(const T*, char*, const char*)) : getname((fntext)v) {}
	};
	constexpr explicit operator bool() const { return title || value.id || value.child; }
	int					width;
	const char*			title;
	element				value;
	int					param;
	proci				proc;
	cmdi				cmd;
	propi				prop;
	//
	bool				action(const char* id, void* object) const;
	bool				isfield() const { return value.id != 0; }
	static const char*	getname(const void* object, char* result, const char* result_maximum) { return ((markup*)object)->title; }
	int					getcount() const;
	int					getcount(const char* id, const void* object, bool need_child) const;
	const markup*		getform(const void* object, const char* id) const { return find(id, object, 0, true); }
	const markup*		getpage(const void* object, int index) const { return find("page", object, index, true); }
	int					getpagecount(const void* object) const { return getcount("page", object, true); }
	const markup*		find(const char* id, const void* object, int result, bool need_child) const;
	const markup*		findcommands(const void* object) const { return find("commands", object, 0, true); }
};