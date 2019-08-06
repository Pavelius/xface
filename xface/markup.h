#pragma once

typedef bool(*allowproc)(const void* object, int index); // Is allow some property
typedef void(*changeproc)(void* object, const void* previous_object); // Change object
typedef void(*commandproc)(void* object); // Object's actions
typedef int(*drawproc)(int x, int y, int width, const void* object); // Custom draw
typedef int(*numproc)(const void* object); // Get object numeric properties
typedef const char* (*textproc)(const void* object, char* result, const char* result_maximum);

// Standart markup
struct markup {
	struct element {
		const char*		id;			// Field identificator (0 for group)
		int				index;		// Array index
		const markup*	child;		// Group or next field
	};
	struct cmdi {
		commandproc		execute;
		changeproc		change;
		constexpr cmdi() : execute(0), change(0) {}
		template<class T> constexpr cmdi(void(*v)(T*)) : execute((commandproc)v), change(0) {}
		template<class T> constexpr cmdi(void(*v)(T*, const T*)) : execute(0), change((changeproc)v) {}
	};
	struct proci {
		allowproc		isallow;	// Is allow special element or command
		allowproc		isvisible;	// Is element visible
		drawproc		custom;
	};
	struct propi {
		textproc		getname;
		numproc			getvalue;
		constexpr propi() : getname(0), getvalue(0) {}
		template<class T> constexpr propi(void(*v)(const T*, char*, const char*)) : getname((textproc)v), getvalue(0) {}
		template<class T> constexpr propi(int(*v)(const T*)) : getname(0), getvalue((numproc)v) {}
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
	int					getcount(const char* id, const void* object, bool need_child) const;
	const markup*		getform(const void* object, const char* id) const { return find(id, object, 0, true); }
	const markup*		getpage(const void* object, int index) const { return find("page", object, index, true); }
	int					getpagecount(const void* object) const { return getcount("page", object, true); }
	const markup*		find(const char* id, const void* object, int result, bool need_child) const;
	const markup*		findcommands(const void* object) const { return find("commands", object, 0, true); }
};