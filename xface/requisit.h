#pragma once

namespace compiler {
struct archive {
	struct header {
		char		signature[4];
		char		version[4];
		void		set(const char* v) { signature[0] = v[0]; signature[1] = v[1]; signature[2] = v[2]; signature[3] = 0; }
		void		set(int v1, int v2) { version[0] = v1 + 0x30; version[1] = '.'; version[2] = v2 + 0x30; version[3] = 0; }
	};
	struct placement {
		unsigned	offset;
		unsigned	count;
	};
	struct file : header {
		placement	strings;
		placement	requisits;
	};
};
struct requisit {
	const char*		id;
	requisit*		parent;
	requisit*		type;
	unsigned		count;
	void* operator new(unsigned size);
	void operator delete(void* ptr, unsigned size);
	requisit*		add(const char* id, requisit* type);
	static requisit* add(const char* id);
	void			clear();
	requisit*		dereference() const;
	bool			isobject() const;
	requisit*		reference() const;
};
extern requisit		number[];
extern requisit		object[];
extern requisit		pointer[];
extern requisit		text[];
}