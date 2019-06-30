#pragma once

// Standart markup
struct markup {
	typedef const char* (*text_type)(const void* object, char* result, const char* result_maximum);
	typedef bool(*allow_type)(const void* object, int index);
	typedef int(*custom_type)(int x, int y, int width, const void* object); // Custom draw
	typedef void(*command_type)(void* object);
	typedef void(*change_type)(void* object, const void* previous_object);
	struct element {
		const char*		id;		// Field identificator (0 for group)
		int				index;	// Array index
		const markup*	child;	// Group or next field
	};
	struct cmdi {
		command_type	execute;
		change_type		change;
		constexpr cmdi() : execute(0), change(0) {}
		template<class T> constexpr cmdi(void(*v)(T*)) : execute((command_type)v), change(0) {}
		template<class T> constexpr cmdi(void(*v)(T*, const T*)) : execute(0), change((change_type)v) {}
	};
	struct proci {
		allow_type		isallow;	// Is allow special element or command
		allow_type		isvisible;	// Is element visible
		custom_type		custom;
	};
	struct propi {
		text_type		getname;
		constexpr propi() : getname(0) {}
		template<class T> constexpr propi(const char* (*v)(const T*, char*, const char*)) : getname((text_type)v) {}
	};
	constexpr explicit operator bool() const { return title || value.id || value.child || cmd.execute; }
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
	int					getcount(const char* id, const void* object, bool need_child) const;
	const markup*		getform(const void* object, const char* id) const { return find(id, object, 0, true); }
	const markup*		getpage(const void* object, int index) const { return find("page", object, index, true); }
	int					getpagecount(const void* object) const { return getcount("page", object, true); }
	const markup*		find(const char* id, const void* object, int result, bool need_child) const;
	const markup*		findcommands(const void* object) const { return find("commands", object, 0, true); }
};