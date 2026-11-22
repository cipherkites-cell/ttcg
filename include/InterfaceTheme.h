#pragma once
#include <array>
#include <string_view>

namespace ttcg {
struct InterfaceTheme {
    std::string_view id, name;
    int boardSurface;
    bool retired = false;
};
// The index is saved in TTCG.ini. Append themes; never reorder or remove entries. A retired theme
// keeps its place so the indices after it stay put, and a saved retired index opens the default.
inline constexpr std::array interfaceThemes{
    InterfaceTheme{"tessera", "Tessera", 3},      InterfaceTheme{"charcoal", "炭灰", 2, true},
    InterfaceTheme{"oxblood", "牛血红", 5, true}, InterfaceTheme{"parchment", "羊皮纸", 4, true},
    InterfaceTheme{"umber", "赭褐", 6, true},     InterfaceTheme{"oathvein", "誓脉", 5},
    InterfaceTheme{"veldun", "维尔敦", 5},        InterfaceTheme{"norden", "诺登", 2},
    InterfaceTheme{"untarnished", "不染", 7},     InterfaceTheme{"edge", "棱边", 5},
    InterfaceTheme{"deardiary", "暗色日记", 2},   InterfaceTheme{"nordic", "北境", 7},
    InterfaceTheme{"outlander", "外域", 2},       InterfaceTheme{"skyrim", "天际", 7},
    InterfaceTheme{"diarypaper", "日记", 5},      InterfaceTheme{"woodpaper", "木与纸", 6}};
inline constexpr int interfaceThemeCount = int(interfaceThemes.size());
inline constexpr int defaultInterfaceTheme = 13;
inline constexpr bool interfaceThemeAvailable(int value) {
    return value >= 0 && value < interfaceThemeCount && !interfaceThemes[value].retired;
}
inline constexpr int interfaceThemeIndex(int value) {
    return interfaceThemeAvailable(value) ? value : defaultInterfaceTheme;
}
static_assert(interfaceThemes[defaultInterfaceTheme].id == "skyrim");
}
