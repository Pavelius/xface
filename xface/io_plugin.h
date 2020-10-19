#include "io.h"
#include "stringbuilder.h"

#pragma once

namespace io {
// Network protocols
enum protocols { NoProtocol, TCP, UDP };
// Application defined reader.
// Plugin read file and post events to this class.
struct plugin {
	const char*			name;
	const char*			fullname;
	const char*			filter;
	static plugin*		first;
	plugin*				next;
	//
	plugin();
	static void			addfilter(stringbuilder& sb, const char* name, const char* find_path);
	static plugin*		find(const char* name);
	static void			getfilter(stringbuilder& sb);
	virtual const char*	read(const char* source, serializer::reader& r) = 0;
	virtual serializer*	write(stream& e) = 0;
};
struct strategy : public serializer::reader {
	const char*			id; // Second level strategy name. If we have root level 'Setting' this can be 'Columns' or 'Windows'.
	const char*			type; // First level 'Root Name', first level xml record etc. Like 'Settings'.
	static strategy*	first;
	strategy*			next;
	//
	strategy(const char* id, const char* type);
	static strategy*	find(const char* name);
	static bool			istrue(const char* name);
	virtual void		write(serializer& e, void* param) = 0;
};
struct address {
	unsigned short		family;
	unsigned short		port;
	unsigned			ip;
	char    			reserved[8];
	//
	void				clear();
	bool				parse(const char* url, const char* service_name);
	bool				tostring(char* node, int node_len, char* service, int service_len);
};
struct socket : public stream, public address {
	socket();
	~socket();
	//
	void				accept(socket& client);
	bool				bind();
	bool				create(protocols type);
	bool				connect();
	void				listen(int backlog);
	int					read(void* result, int count) override;
	int					write(const void* result, int count) override;
private:
	int					s;
};
bool					read(const char* url, serializer::reader& e);
bool					read(const char* url, const char* strategy_type, void* param);
bool					write(const char* url, const char* strategy_type, void* param);
}