#include "Datetime.h"
#include <map>
#include <fmt/format.h>
#include <fmt/chrono.h>

std::string pluralize(std::string word) {
    switch (word.at(word.length() - 1)) {
    case 'r':
    case 'l':
        return word;
        break;
    case 'e': {
        const auto before_ending = word.at(word.length() - 2);
        switch (before_ending) {
        case 'm':
            return word.substr(0, word.length() - 1) + "ar";
            break;
        case 'd':
        case 'p':
            return word + "n";
            break;
        case 'r':
            return word;
            break;
        }
        return word.substr(0, word.length() - 1) + "ar";
    }
        break;
    case 'g':
    case 'p':
        return word + "ar";
        break;
    case 'd':
    case 't':
        return word + "er";
        break;
    case 'a':
        return word.substr(0, word.length() - 1) + "or";
        break;
    }

    return word;
}

wgr::DateInterval::DateInterval(int y, int m, int d, int h, int i, int s, bool invert)
        : m_years(y), m_months(m), m_days(d), m_hours(h), m_minutes(i), m_seconds(s), m_invert(invert) {}

wgr::DateInterval time_diff(const wgr::Datetime *one, const wgr::Datetime *two) {
    wgr::DateInterval rt{ };
    if (one->sse() > two->sse()) {
        auto *tmp = two;
        two = one;
        one = tmp;
        rt.set_inverted(true);
    }
    auto *base = one;

    auto range_limit = [](int start, int end, int adj, int *a, int *b) {
        if (*a < start) {
            *b -= (start - *a - 1) / adj + 1;
            *a += adj * ((start - *a - 1) / adj + 1);
        }
        if (*a >= end) {
            *b += *a / adj;
            *a -= adj * (*a / adj);
        }
    };

    rt.set_years(two->tm().tm_year - one->tm().tm_year);
    rt.set_months(two->tm().tm_mon - one->tm().tm_mon);
    rt.set_days(two->tm().tm_mday - one->tm().tm_mday);
    rt.set_hours(two->tm().tm_hour - one->tm().tm_hour);
    rt.set_minutes(two->tm().tm_min - one->tm().tm_min);
    rt.set_seconds(two->tm().tm_sec - one->tm().tm_sec);

    range_limit(0, 60, 60, &rt.seconds(), &rt.minutes());
    range_limit(0, 60, 60, &rt.minutes(), &rt.hours());
    range_limit(0, 24, 24, &rt.hours(), &rt.days());
    range_limit(0, 12, 12, &rt.months(), &rt.years());

    //range_limit_days(&base->tm().tm_year, &base->tm().tm_year, &rt.years(), &rt.months(), &rt.days(), rt.inverted());
    //range_limit(0, 12, 12, &rt.months(), &rt.years());
    return rt;
}

wgr::DateInterval wgr::Datetime::diff(Datetime other) const {
    return time_diff(this, &other);
}

std::string wgr::Datetime::format() const {
    return fmt::format(
            "{:%Y-%m-%d %H:%M:%S}",
            m_tm
    );
}

wgr::Datetime wgr::Datetime::now() {
    time_t t{ };
    std::time(&t);
    return from_time_t(t);
}

template<typename Pair>
struct SecondPair {
    typename Pair::second_type operator()(const Pair &p) const { return p.second; }
};

template<typename Map>
SecondPair<typename Map::value_type> second_pair(const Map &m) { return SecondPair<typename Map::value_type>(); }

std::string wgr::Datetime::relative_format(const Datetime &start, const Datetime &end) {
    static std::map<std::string, std::string> phrases{
            { "ago", "ago" },
            { "and", "och" },
            { "now", "just now" },
            { "y", "year" },
            { "m", "month" },
            { "d", "day" },
            { "h", "hour" },
            { "i", "minute" },
            { "s", "second" },
    };

    auto diff = end.diff(start);
    enum class Period {
        Y, // years
        M, // months
        D, // days
        H, // hours
        I, // minutes
        S, // seconds
    };

    std::map<Period, std::string> final_data{
            { Period::Y, phrases["y"] },
            { Period::M, phrases["m"] },
            { Period::D, phrases["d"] },
            { Period::H, phrases["h"] },
            { Period::I, phrases["i"] },
            { Period::S, phrases["s"] },
    };

    auto check_if_keep = [&](int v, Period p) {
        if (v == 0) {
            final_data.erase(p);
            return;
        }

        final_data.at(p) = fmt::format(
                "{} {}",
                v,
                v > 1 ? pluralize(final_data.at(p)) : final_data.at(p)
        );
    };

    check_if_keep(diff.years(), Period::Y);
    check_if_keep(diff.months(), Period::M);
    check_if_keep(diff.days(), Period::D);
    check_if_keep(diff.hours(), Period::H);
    check_if_keep(diff.minutes(), Period::I);
    check_if_keep(diff.seconds(), Period::S);

    if (final_data.empty()) {
        return phrases["now"];
    }

    std::string final_string{ };
    std::for_each(
            final_data.begin(), final_data.end(), [&](auto &pair) {
                if (!final_string.empty()) {
                    final_string += ", ";
                }
                final_string += pair.second;
            }
    );

    if (!diff.inverted()) {
        final_string += " " + phrases["ago"];
    }

    return final_string;
}

wgr::Datetime wgr::Datetime::from_time_t(time_t time) {
    struct tm tm{ };
    if (::localtime_s(&tm, &time)) {
        return Datetime{ };
    }

    return Datetime{ time, tm };
}

