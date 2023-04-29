#pragma once
#include <chrono>

namespace yui {

template<typename Clock = std::chrono::steady_clock>
class FrameTimer {
public:
    using TimePoint = typename Clock::time_point;

public:
    FrameTimer()
            : m_last(Clock::now()),
              m_duration(),
              m_cached_progress(0.0f),
              m_frame_count(0u),
              m_frames_per_second(0u) {}

    float update() {
        ++m_frame_count;
        auto tmp = m_last;
        m_last = Clock::now();
        m_duration = std::chrono::duration_cast<std::chrono::duration<float>>(m_last - tmp);
        auto dt = m_duration.count();
        m_cached_progress += dt;
        if (m_cached_progress >= 1.0f) {
            m_frames_per_second = m_frame_count;
            m_frame_count = 0;
            m_cached_progress = 0.0f;
        }
        return dt;
    }

    [[nodiscard]] const std::chrono::duration<float> &get_duration() const {
        return m_duration;
    }

    const auto &get_last() const {
        return m_last;
    }

    [[nodiscard]] uint32_t get_frames_per_second() const {
        return m_frames_per_second;
    }

private:
    TimePoint m_last{ };
    std::chrono::duration<float> m_duration{ };
    float m_cached_progress{ 0.f };
    uint32_t m_frame_count{ },
            m_frames_per_second;
};

}
