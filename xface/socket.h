#pragma once

namespace io {
class sock {
	int					s;
public:
	constexpr sock() : s(-1) {}
	~sock() { close(); }
	constexpr explicit operator bool() const { return s != -1; }
	void				close();
	int					createserver(const char* port);
	static int			initialize();
	static int			getlasterror();
	int					open(const char* url, const char* port);
	int					read(char* buffer, unsigned size);
	int					stopwrite();
	int					stopread();
	int					waitclient(sock& client) const;
	int					write(const char* buffer, unsigned size);
};
}