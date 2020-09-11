#include "crt.h"

#pragma once

class codemodel : arem<char> {
	int						p1, p2;
public:
	void					clear();
	void					correct();
	int						getbegin() const;
	int						getend() const;
	void					paste(const char* input);
	void					select(int i, bool shift);
	virtual void			invalidate() {}
};