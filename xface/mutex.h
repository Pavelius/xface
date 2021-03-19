#pragma once

namespace io {
class mutex {
	int			s;
public:
	mutex();
	~mutex();
	constexpr explicit operator bool() const { return s != 0; }
	void		lock();
	void		unlock();
};
}
