#pragma once

class datetime {
	unsigned				d;
public:
	constexpr datetime() : d(0) {}
	constexpr datetime(unsigned d) : d(d) {}
	constexpr operator unsigned() const { return d; }
	constexpr explicit operator bool() const { return d != 0; }
	constexpr bool operator==(const datetime& e) const { return d == e.d; }
	datetime(int year, int month, int day, int hour, int minute);
	int						day() const;
	datetime				daybegin() const { return datetime(year(), month(), day(), 0, 0); }
	datetime				dayend() const { return datetime(year(), month(), day(), 23, 59); }
	constexpr int			hour() const { return (d / 60) % 24; }
	bool					isleap() const;
	constexpr int			minute() const { return d % 60; }
	int						month() const;
	datetime				monthbegin() const { return datetime(year(), month(), 1, 0, 0); }
	static datetime			now();
	int						year() const;
	datetime				yearbegin() const { return datetime(year(), 1, 1, 0, 0); }
};