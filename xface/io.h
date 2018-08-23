#pragma once

enum seek_flags { SeekSet, SeekCur, SeekEnd };
enum stream_flags {
	StreamRead = 1,
	StreamWrite = 2,
	StreamText = 4,
};

namespace io {
// Network protocols
enum protocols { NoProtocol, TCP, UDP };
// Abstract bi-stream interface
struct stream {
	stream&				operator<<(const char* t); // Post text string into stream data in correct coding.
	stream&				operator<<(const int n); // Post number as string into stream data in correct coding.
	unsigned char		get();
	unsigned short		getLE16();
	unsigned			getLE32();
	unsigned			getsize();
	void				gettext(char* result, int max_buffer);
	virtual int			read(void* result, int count) = 0;
	void				read(bool& value) { read(&value, sizeof(value)); }
	void				read(int& value) { read(&value, sizeof(value)); }
	void				read(unsigned& value) { read(&value, sizeof(value)); }
	void				read(short& value) { read(&value, sizeof(value)); }
	void				read(unsigned short& value) { read(&value, sizeof(value)); }
	void				read(unsigned char& value) { read(&value, sizeof(value)); }
	template<class T> void read(T& object);
	int					readsig(char* signature) { return read(signature, 4); }
	void				puttext(const char* text);
	virtual int			seek(int count, int rel = SeekCur) { return 0; };
	virtual int			write(const void* result, int count) = 0;
	void				write(bool value) { write(&value, sizeof(value)); }
	void				write(int value) { write(&value, sizeof(value)); }
	void				write(unsigned value) { write(&value, sizeof(value)); }
	void				write(short value) { write(&value, sizeof(value)); }
	void				write(unsigned short value) { write(&value, sizeof(value)); }
	void				write(unsigned char value) { write(&value, sizeof(value)); }
	template<class T> void write(const T& e); // For overloaded stream output
	void				writescan(void* p, int width, int heght, int scan_line, int element_size);
	int					writesig(const char* signature) { return write(signature, 4); }
};
struct sequence : public stream {
	int					read(void* result, int count) override;
	int					write(const void* result, int count) override;
	int					seek(int count, int rel = SeekCur) override;
	bool				match(const char* value);
	sequence(io::stream& parent);
private:
	char				cashed[64];
	int					cashed_count;
	io::stream&			parent;
	//
	void				makecashe(int count);
};
struct file : public stream {
	struct find {
		find(const char* url);
		~find();
		const char*		name();
		const char*		fullname(char* result);
		void			next();
		operator bool() const { return handle != 0; }
	private:
		char			path[261];
		char			reserved[512];
		void*			handle;
	};
	file();
	file(const char* url, unsigned flags = StreamRead);
	~file();
	operator bool() const { return handle != 0; }
	void				close();
	bool				create(const char* url, unsigned flags);
	static bool			exist(const char* url);
	static char*		getdir(char* url, int size);
	static char*		getmodule(char* url, int size);
	static bool			makedir(const char* url);
	int					read(void* result, int count) override;
	static bool			remove(const char* url);
	int					seek(int count, int rel) override;
	static bool			setdir(const char* url);
	int					write(const void* result, int count) override;
private:
	int					handle;
};
struct memory : public stream {
	memory(void* data, int size);
	int					read(void* result, int count) override;
	int					seek(int count, int rel) override;
	int					write(const void* result, int count) override;
private:
	unsigned char*		data;
	int					pos;
	int					size;
};
struct node {
	node*				parent;
	const char*			name;
	int					type, index;
	bool				skip; // set this if you want skip block
	int					params[12];
	//
	node(int type = 0);
	node(node& parent, const char* name = "", int type = 0);
	bool				operator==(const char* name) const;
	//
	int					getlevel() const;
};
// Application defined reader.
// Plugin read file and post events to this class.
struct reader {
	virtual void		open(node& e) {}
	virtual void		set(node& e, int value) {};
	virtual void		set(node& e, const char* value) {};
	virtual void		close(node& e) {}
};
// Application create instance of this object.
// Then write data use custom, application-defined logic.
struct writer {
	stream&				e;
	writer(stream& e) : e(e) {}
	virtual ~writer() {}
	virtual void		open(const char* name, int type = 0) {}
	virtual void		set(const char* name, int value, int type = 0) {};
	virtual void		set(const char* name, const char* value, int type = 0) {};
	virtual void		close(const char* name, int type = 0) {}
};
struct plugin {
	const char*			name;
	const char*			fullname;
	const char*			filter;
	static plugin*		first;
	plugin*				next;
	//
	plugin();
	static plugin*		find(const char* name);
	static char*		getfilter(char* result, const char* result_maximum);
	virtual const char*	read(const char* source, reader& r) = 0;
	virtual writer*		write(stream& e) = 0;
};
struct strategy : public reader {
	const char*			id; // Second level strategy name. If we have root level 'Setting' this can be 'Columns' or 'Windows'.
	const char*			type; // First level 'Root Name', first level xml record etc. Like 'Settings'.
	static strategy*	first;
	strategy*			next;
	//
	strategy(const char* id, const char* type);
	static strategy*	find(const char* name);
	virtual void		write(io::writer& e, void* param) = 0;
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
bool					read(const char* url, io::reader& e);
bool					read(const char* url, const char* strategy_type, void* param);
bool					write(const char* url, const char* strategy_type, void* param);
}