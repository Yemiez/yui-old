#pragma once
#include <chrono>
#include <string>
#include <utility>
#include <vector>
#include <ctime>
#include <iostream>
#include <fstream>

namespace yui::io {

	template <typename TimeType = std::chrono::milliseconds, typename ClockType = std::chrono::high_resolution_clock>
	class Profiler
	{
	public:
		class Timer
		{
		private:
			using TimePoint = std::chrono::time_point<ClockType>;
			
		public:
			Timer() = default;

			void tic()
			{
				m_start = ClockType::now();
			}

			void toc()
			{
				m_end = ClockType::now();
			}

			std::chrono::duration<float> duration() const { return std::chrono::duration_cast<TimeType>(m_end - m_start); }
		private:
			TimePoint m_start{},
				m_end{};
		};
		
		class Profile
		{
		public:
			Profile(std::string name)
				: m_name(std::move(name)), m_timings()
			{}

			const std::string& name() const { return m_name; }
			const std::vector<std::pair<float, std::time_t>>& timings() const { return m_timings; }

			float average() const
			{
				if (timings().empty()) return 0.f;

				auto sum = 0.f;
				for (const auto& [t, _] : timings())
					sum += t;
				
				return sum / timings().size();
			}

			float highest() const
			{
				return m_highest;
			}

			// Add a timing
			std::pair<float, std::time_t> add_sample(float duration)
			{
				if (duration > m_highest) {
					m_highest = duration;
				}
				
				return m_timings.emplace_back(duration, std::time(nullptr));
			}
			
		private:
			std::string m_name;
			std::vector<std::pair<float, std::time_t>> m_timings;
			float m_highest{0.0f};
		};
		
	public:
		Profiler() = default;
		Profiler(Profiler&&) = default;
		Profiler(const Profiler&) = delete;
		~Profiler() noexcept
		{
			if (m_file) {
				try {
					m_file->close();
				} catch(...) {}
			}
		}

		template<typename Callable, typename...Arguments>
		void run(const std::string_view& experiment_name, Callable&& callable, Arguments&&...args)
		{
			// Collect sample
			Timer timer;
			timer.tic();
			callable(std::forward<Arguments>(args)...);
			timer.toc();
			
			auto& profile = get_profile(experiment_name);
			const auto& sample = profile.add_sample(timer.duration().count());
			on_new_sample(profile, sample);
		}

		Profile& get_profile(std::string_view name)
		{
			for (auto &profile : m_profiles) {
				if (profile.name() == name) {
					return profile;
				}
			}

			return m_profiles.emplace_back(name.data());
		}

		const Profile& get_profile(std::string_view name) const
		{
			for (auto &profile : m_profiles) {
				if (profile.name() == name) {
					return profile;
				}
			}

			throw std::exception("Profile does not exist");
		}

		const std::vector<Profile>& profiles() const { return m_profiles; }
		
		void attach_file(const std::shared_ptr<std::ofstream>& file)
		{
			m_file = file;
		}

		void report(std::basic_ostream<char>& output_stream, bool report_individual_samples = false)
		{
			for (auto &profile : m_profiles) {
				output_stream << "Profile> " << profile.name() << " | Avg: " << profile.average() << " | Samples: " << profile.timings().size() << "\n";

				if (report_individual_samples) {
					for (auto &[sample, time] : profile.timings()) {
						output_stream << "\t" << sample << " seconds @ " << time << "\n";
					}	
				}

				output_stream << "    ^ Highest: " << profile.highest() << " seconds\n";
				output_stream << "\n";
			}
		}

	private:
		void on_new_sample(const Profile& profile, const std::pair<float, std::time_t> &sample)
		{
			if (!(m_file && m_file->is_open())) 
				return;
			*m_file << profile.name() << " | sample: " << sample.first << " seconds @ " << sample.second << "\n";
			m_file->flush();
		}
		
	private:
		std::vector<Profile> m_profiles;
		std::shared_ptr<std::ofstream> m_file;
	};

}
