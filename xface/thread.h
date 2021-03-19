#pragma once

namespace io {
class thread {
	int				s;
public:
	typedef void (*fnroutine)(void* v);
	thread(fnroutine proc, void* param);
	~thread();
	void			join();
};
}
