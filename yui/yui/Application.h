#pragma once
#include <map>
#include <string>
#include <vector>
#include "includes.h"
#include "FrameTimer.h"
#include "io/Profiler.h"

#define BENCHMARK

#ifdef BENCHMARK
#define BENCHMARK_BEGIN yui::Application::the().profiler().run(__FUNCTION__, [&]() {
#define BENCHMARK_END });
#else
#define BENCHMARK_BEGIN
#define BENCHMARK_END
#endif

struct GLFWwindow;
namespace yui {
class Window;

class Application {
private:
    struct WindowInstance;

public:
    Application(int, char **);

    int exec();

    FT_Library freetype() { return m_freetype_library; }

    [[nodiscard]] bool halting() const { return m_halting; }
    void halt(int exit_code = 0);

    void register_window(yui::Window *);
    //void unregister_window(yui::Window*);

    void report_error(const std::string &);

    static bool initialized();
    static Application &the();

    void window_resize(Window *, int x, int y);
    void window_refresh(Window *);

    io::Profiler<> &profiler() { return m_profiler; }
private:
    void do_window_refresh(WindowInstance &);

private:
    struct WindowInstance {
        uint32_t id{ };
        Window *window{ };
        GLFWwindow *glfw{ };
        std::map<uint32_t, bool> flags{ };
        FrameTimer<> timer{ };
        float passed_time{ 0.f };
        bool initialized{ false };
        int fps{ 0 };
        std::string original_title{ };
    };

    std::vector<std::string> m_arguments{ };
    bool m_halting{ false };
    int m_exit_code{ 0 };
    std::map<uint32_t, WindowInstance> m_window_map{ };
    FT_Library m_freetype_library{ };
    uint32_t m_window_counter{ 0 };
    io::Profiler<> m_profiler{ };
};
}
