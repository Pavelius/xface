#include "anyval.h"
#include "rect.h"

#pragma once

struct wizard {
	enum command_s : unsigned char {
		Try, Draw, Initialize
	};
	struct proct {
		typedef bool (wizard::*callp)(const rect& rc, command_s id);
		callp		call;
		template<class T> proct(bool (T::*v)(const rect& rc, command_s id)) : call((callp)v) {}
	};
	struct element {
		const char*		title;
		proct			proc;
		const char*		image;
		explicit operator bool() const { return title != 0; }
	};
	static void			choosefolder(const anyval& v);
	virtual const element* getelements() const = 0;
	const element*		getlast(const element* p) const;
	const element*		getvalid(const element* p) const;
	void				show(const char* title);
};
