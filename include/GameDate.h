#pragma once
#include <array>
#include <algorithm>
#include <string>
#include <format>
namespace ttcg {
struct GameDate {
    int year = 201, month = 7, day = 17, hour = 10;
};
inline GameDate shiftGameDate(GameDate date, int hours) {
    constexpr std::array<int, 12> days{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    date.month = std::clamp(date.month, 0, 11);
    date.day = std::clamp(date.day, 1, days[date.month]);
    const int total = date.hour + hours;
    const int offset = total >= 0 ? total / 24 : (total - 23) / 24;
    date.hour = total - offset * 24;
    date.day += offset;
    while (date.day > days[date.month]) {
        date.day -= days[date.month];
        if (++date.month == 12) {
            date.month = 0;
            ++date.year;
        }
    }
    while (date.day < 1) {
        if (--date.month < 0) {
            date.month = 11;
            --date.year;
        }
        date.day += days[date.month];
    }
    return date;
}
inline std::string gameDateText(GameDate date) {
    constexpr const char* months[]{"晨星", "日晓", "初种", "雨手", "二种", "年中",
                                   "日高", "末种", "炉火", "霜落", "日暮", "晚星"};
    return std::format("4E {} 年 {}月 {} 日 · {:02}:00", date.year, months[date.month], date.day, date.hour);
}
}
