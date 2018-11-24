#pragma once

struct datetime {
	constexpr datetime() : d(0) {}
	constexpr datetime(unsigned m_d) : d(m_d) {}
	constexpr unsigned		gethour(unsigned d) { return (d / 60) % 24; } // Get hour
	constexpr unsigned		getminute(unsigned d) { return (d % 60); } // Get minute
private:
	unsigned				d;
};
