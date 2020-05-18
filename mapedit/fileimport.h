#pragma once

class fileimport {
	const char*			name;
	void*				data;
	unsigned			size;
	bool				need_serial;
	const char*			geturl(char* buffer) const;
public:
	fileimport(const char* name, void* data, unsigned size);
	~fileimport();
	void				setnoserial() { need_serial = false; }
};
