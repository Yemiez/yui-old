#pragma once
#include <chrono>

namespace wgr {

	class DateInterval
	{
	public:
		DateInterval() = default;
		DateInterval(int y, int m, int d, int h, int i, int s, bool invert);

		int years() const { return m_years; }
		int months() const { return m_months; }
		int days() const { return m_days; }
		int hours() const { return m_hours; }
		int minutes() const { return m_minutes; }
		int seconds() const { return m_seconds; }
		int& years() { return m_years; }
		int& months() { return m_months; }
		int& days() { return m_days; }
		int& hours() { return m_hours; }
		int& minutes() { return m_minutes; }
		int& seconds() { return m_seconds; }
		bool inverted() const { return m_invert; }
	public:
		void set_years(int y) { m_years = y; }
		void set_months(int m) { m_months = m; }
		void set_days(int d) { m_days = d; }
		void set_hours(int h) { m_hours = h; }
		void set_minutes(int i) { m_minutes = i; }
		void set_seconds(int s) { m_seconds = s; }
		void set_inverted(bool v) { m_invert = v; }
	private:
		int m_years{};
		int m_months{};
		int m_days{};
		int m_hours{};
		int m_minutes{};
		int m_seconds{};
		bool m_invert{};
	};
	
	class Datetime
	{
	public:
		using Clock = std::chrono::high_resolution_clock;
		using TimePoint = std::chrono::time_point<Clock, std::chrono::duration<int64_t>>;

	public:
		Datetime() = default;
		Datetime(time_t time, std::tm tm)
			: m_sse(time), m_tm(tm)
		{}

		time_t sse() const { return m_sse; }
		const std::tm& tm() const { return m_tm; }
		
		//Datetime diff(Datetime other) const;
		DateInterval diff(Datetime other) const;

		///	Formats a datetime object (see fmt.dev syntax for std::tm object).
		/// {0} = Long year (yyyy)
		///	{1} = short year (yy)
		///	{2} = month
		///	{3} = day 
		///	{4} = hour
		///	{5} = minute 
		///	{6} = second
		std::string format(const std::string& format = "{:%Y-%m-%d %H:%M:%S}");

		static Datetime now();
		static std::string relative_format(const Datetime& start, const Datetime& end);
		static Datetime from_time_t(time_t);

	private:
		static time_t to_time(TimePoint);
	private:
		time_t m_sse{}; // seconds since epoch
		std::tm m_tm{}; // time structure
	};
	
}
