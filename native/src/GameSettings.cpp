#include "GameApp.h"
#include "ChineseText.h"
#include <cmath>
#include <cstdio>
#include <numbers>
#ifdef _WIN32
#include <windows.h>
#endif
namespace preview {
namespace {
std::string keyLabel(unsigned binding) {
    if (!binding) return "未绑定";
    std::string text;
    for (auto [mask, name] : std::array<std::pair<unsigned, const char*>, 6>{
             {{256, "LShift"}, {512, "RShift"}, {1024, "LCtrl"}, {2048, "RCtrl"}, {4096, "LAlt"}, {8192, "RAlt"}}})
        if (binding & mask) text += std::string(name) + " + ";
    const unsigned scan = binding & 255;
#ifdef _WIN32
    wchar_t wide[64]{};
    const LONG parameter = (scan & 127) << 16 | (scan >= 128 ? 1 << 24 : 0);
    if (GetKeyNameTextW(parameter, wide, 64)) {
        char result[192]{};
        WideCharToMultiByte(CP_UTF8, 0, wide, -1, result, 192, nullptr, nullptr);
        return text + result;
    }
#endif
    if (scan >= 59 && scan <= 68) return text + "F" + std::to_string(scan - 58);
    return text + "按键 " + std::to_string(scan);
}
}
void GameApp::matchAppearancePreview(ImVec2 at, ImVec2 size) {
    if (collection.cards.empty()) return;
    auto* d = ImGui::GetWindowDrawList();
    const float gap = 18 * scale;
    line(at - ImVec2(AlbumLayout::gap * .5f * scale, 0), at + ImVec2(-AlbumLayout::gap * .5f * scale, size.y));
    const float w = std::min((size.x - gap - 8 * scale) * .5f, std::max(54 * scale, (size.y - 280 * scale) * .375f)),
                h = w / .75f;
    const float left = (size.x - 2 * w - gap) * .5f;
    // Reuse known art so the preview doesn't reveal undiscovered cards.
    const auto known = [&](int i) {
        const auto& c = collection.cards[i];
        return c.known[0] || c.known[1] || c.owned[0] || c.owned[1];
    };
    int sample = std::clamp(selected, 0, int(collection.cards.size()) - 1);
    if (!known(sample)) {
        sample = -1;
        for (int i = 0; i < int(collection.cards.size()); ++i)
            if (known(i)) {
                sample = i;
                break;
            }
    }
    if (sample < 0) return;
    const auto handAt = at + ImVec2(left, 76 * scale);
    text(at + ImVec2(size.x * .5f, 0), "手牌", 18, palette().muted, 0, true);
    for (int owner = 0; owner < 2; ++owner) {
        const auto p = handAt + ImVec2(owner * (w + gap), 0);
        text(p + ImVec2(w * .5f, -33 * scale), owner ? "对手" : "你", 18, controlColour(owner, true), 0, true);
        card(sample, p, ImVec2(w, h), false, false, owner, MaterialSlot::appearance + owner, false, CardContext::Hand);
    }
    const auto boardAt = handAt + ImVec2(0, h + 82 * scale);
    text(boardAt + ImVec2(w + gap * .5f, -39 * scale), "牌桌", 18, palette().muted, 0, true);
    const auto material = number(member(state, "settings"), "boardSurface");
    boardSurface(material, boardAt - ImVec2(7, 7) * scale, ImVec2(2 * w + gap, h) + ImVec2(14, 14) * scale);
    card(sample, boardAt, ImVec2(w, h), false, false, 0, MaterialSlot::appearance + 2, false, CardContext::Board);
    float fold = 1;
    int owner = previewOwner;
    if (previewCapturedAt >= 0) {
        const double progress = (ImGui::GetTime() - previewCapturedAt) / .55;
        if (progress >= 1) {
            previewOwner = 1 - previewOwner;
            owner = previewOwner;
            previewCapturedAt = -1;
        } else {
            if (progress >= .5) owner = 1 - previewOwner;
            fold = std::max(.025f, float(std::abs(std::cos(progress * std::numbers::pi))));
        }
    }
    const auto cardAt = boardAt + ImVec2(w + gap, 0);
    const int first = d->VtxBuffer.Size;
    card(sample, cardAt, ImVec2(w, h), false, false, owner, MaterialSlot::appearance + 3, false, CardContext::Board);
    for (int v = first; v < d->VtxBuffer.Size; ++v)
        d->VtxBuffer[v].pos.x = cardAt.x + w * .5f + (d->VtxBuffer[v].pos.x - cardAt.x - w * .5f) * fold;
    const auto controls = boardAt + ImVec2(0, h + 27 * scale);
    if (button("preview-capture", previewOwner ? "夺牌" : "还牌", controls, ImVec2(2 * w + gap, 36 * scale), false,
               previewCapturedAt < 0))
        previewCapturedAt = ImGui::GetTime();
    if (button("preview-next-art", "换一张卡", controls + ImVec2(0, 48 * scale), ImVec2(2 * w + gap, 36 * scale))) {
        for (int i = 0; i < int(collection.cards.size()); ++i) {
            selected = (sample + 1 + i) % int(collection.cards.size());
            if (known(selected)) break;
        }
    }
}
void GameApp::settings(ImVec2 at, ImVec2 size) {
    const auto& s = member(state, "settings");
    const auto& dev = member(s, "development");
    const float nav = AlbumLayout::navigation * scale, gap = AlbumLayout::gap * scale;
    const bool developer = flag(dev, "enabled");
    if (settingsPage == 3 && !developer) settingsPage = 0;
    const char* names[]{"外观", "操作", "玩法", "开发者"};
    for (int i = 0; i < (developer ? 4 : 3); ++i)
        if (chapter(names[i], names[i], at + ImVec2(0, i * 47 * scale), ImVec2(nav - 20 * scale, 43 * scale),
                    settingsPage == i))
            settingsPage = i;
    line(at + ImVec2(nav + gap / 2, 0), at + ImVec2(nav + gap / 2, size.y));
    const float preview = settingsPage == 0 ? AlbumLayout::preview * scale : 0;
    const float area = size.x - nav - gap - (preview ? preview + gap : 0);
    const auto origin = at + ImVec2(nav + gap, 0);
    text(origin, names[settingsPage], 32, palette().ink);
    if (settingsPage == 0) {
        if (sectionTab("appearance-general", "常规", origin + ImVec2(area - 198 * scale, 0), ImVec2(90, 40) * scale,
                       appearancePage == 0))
            appearancePage = 0;
        if (sectionTab("appearance-match", "对局", origin + ImVec2(area - 98 * scale, 0), ImVec2(90, 40) * scale,
                       appearancePage == 1))
            appearancePage = 1;
    }
    ImGui::SetCursorScreenPos(origin + ImVec2(0, 63) * scale);
    ImGui::BeginChild(settingsPage == 0 && appearancePage == 1 ? "match-appearance-body" : "settings-body",
                      ImVec2(area, size.y - 63 * scale), ImGuiChildFlags_NavFlattened);
    const float width = ImGui::GetContentRegionAvail().x - 16 * scale;
    const auto valueOf = [&](const char* json) {
        if (member(s, json).is_boolean()) return int(flag(s, json));
        for (const auto& field : ttcg::InterfaceSettings::fields)
            if (field.json == json) return number(s, json, field.defaultValue);
        return number(s, json);
    };
    const auto heading = [&](const char* title) {
        ImGui::Dummy(ImVec2(0, 15 * scale));
        line(ImGui::GetCursorScreenPos(), ImGui::GetCursorScreenPos() + ImVec2(width, 0));
        ImGui::Dummy(ImVec2(0, 19 * scale));
        textBlock(title, width, 21, palette().accent);
        ImGui::Dummy(ImVec2(0, 16 * scale));
    };
    const auto toggle = [&](const char* name, const char* key, const char* json, const char* off = "关") {
        bool v = valueOf(json) != 0;
        const auto p = ImGui::GetCursorScreenPos();
        text(p + ImVec2(0, 9) * scale, name, 18, palette().ink, width - 115 * scale);
        if (button(key, v ? "开" : off, p + ImVec2(width - 68 * scale, 0), ImVec2(68, 36) * scale, v,
                   !pendingCommand && !capturing()))
            send("settings", std::string(key) + " " + (v ? "0" : "1"));
        ImGui::SetCursorScreenPos(p + ImVec2(0, 58) * scale);
        ImGui::Dummy(ImVec2(width, 0));
    };
    const auto select = [&](const char* name, const char* key, const char* json, std::vector<std::string> choices) {
        int value = valueOf(json);
        if (combo(key, name, value, choices, width)) send("settings", std::string(key) + " " + std::to_string(value));
    };
    const auto slider = [&](const char* name, const char* key, const char* json, int maximum, const char* format = "%d",
                            bool tenths = false, bool skinDefault = false) {
        const auto p = ImGui::GetCursorScreenPos();
        text(p + ImVec2(0, 8) * scale, name, 18, palette().ink, width * .51f);
        ImGui::SetCursorScreenPos(p + ImVec2(width * .56f, 0));
        ImGui::SetNextItemWidth(width * .44f - 72 * scale);
        int value = valueOf(json);
        for (auto colour : {ImGuiCol_FrameBg, ImGuiCol_FrameBgHovered, ImGuiCol_FrameBgActive, ImGuiCol_SliderGrab,
                            ImGuiCol_SliderGrabActive})
            ImGui::PushStyleColor(colour, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
        ImGui::SliderInt((std::string("##") + key).c_str(), &value, 0, maximum, "", ImGuiSliderFlags_NoInput);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(5);
        const auto low = ImGui::GetItemRectMin(), high = ImGui::GetItemRectMax();
        const float y = (low.y + high.y) * .5f;
        const float first = low.x + 8 * scale, last = high.x - 8 * scale,
                    x = first + (last - first) * float(value) / maximum;
        auto* draw = ImGui::GetWindowDrawList();
        draw->AddLine(ImVec2(first, y), ImVec2(last, y), palette().line, 2 * scale);
        draw->AddLine(ImVec2(first, y), ImVec2(x, y), palette().accent, 2 * scale);
        draw->AddCircleFilled(ImVec2(x, y), (ImGui::IsItemHovered() || ImGui::IsItemActive() ? 4.5f : 3.5f) * scale,
                              palette().accent, 20);
        char shown[32]{};
        if (skinDefault)
            std::snprintf(shown, sizeof(shown), "皮肤");
        else if (tenths)
            std::snprintf(shown, sizeof(shown), format, value / 10.f);
        else
            std::snprintf(shown, sizeof(shown), format, value);
        textRight(p + ImVec2(width, 8 * scale), shown, 18, palette().ink);
        if (ImGui::IsItemDeactivatedAfterEdit())
            send("settings", std::string(key) + " " + std::to_string(value));
        else if (ImGui::IsItemActive())
            state["settings"][json] = value;
        ImGui::SetCursorScreenPos(p + ImVec2(0, 58) * scale);
        ImGui::Dummy(ImVec2(width, 0));
    };
    if (settingsPage == 0 && appearancePage == 0) {
        const auto p = ImGui::GetCursorScreenPos();
        const int current = number(s, "windowSize", 90);
        text(p + ImVec2(0, 8) * scale, "全屏", 18, palette().ink);
        if (button("fullscreen", current == 100 ? "开" : "关", p + ImVec2(width - 68 * scale, 0),
                   ImVec2(68, 36) * scale, current == 100))
            send("settings", "window-size " + std::to_string(current == 100 ? 90 : 100));
        ImGui::SetCursorScreenPos(p + ImVec2(0, 58) * scale);
        ImGui::Dummy(ImVec2(width, 0));
        int sizeIndex = std::clamp((current - 60) / 5, 0, 8);
        if (combo("window-size", "窗口大小", sizeIndex,
                  {"60%", "65%", "70%", "75%", "80%", "85%", "90%", "95%", "100%"}, width))
            send("settings", "window-size " + std::to_string(60 + 5 * sizeIndex));
        heading("主题");
        // Retired themes are not offered; the default leads the list.
        std::vector<int> themeIndices{ttcg::defaultInterfaceTheme};
        for (int index = 0; index < ttcg::interfaceThemeCount; ++index)
            if (ttcg::interfaceThemeAvailable(index) && index != ttcg::defaultInterfaceTheme)
                themeIndices.push_back(index);
        std::vector<std::string> themeNames;
        for (int index : themeIndices) themeNames.emplace_back(ttcg::interfaceThemes[index].name);
        int themeChoice = int(std::find(themeIndices.begin(), themeIndices.end(), theme) - themeIndices.begin());
        if (combo("theme", "界面主题", themeChoice, themeNames, width))
            send("settings", "theme " + std::to_string(themeIndices[themeChoice]));
        select("按键提示", "key-prompts", "keyPrompts", {"自动", "皮肤样式", "已安装图标"});
        select("牌桌", "board-surface", "boardSurface",
               {"跟随主题", "素色", "石板", "毛毡", "羊皮纸", "皮革", "胡桃木", "深色石台"});
        slider("牌桌纹理", "board-texture", "boardTexture", 100, "%d%%");
        heading("字体（中文界面统一使用 MiSans）");
        const auto skinFontName = skin().font == "lato"    ? "Lato"
                                  : skin().font == "serif" ? "衬线"
                                  : skin().font == "sans"  ? "无衬线"
                                                           : "Jost";
        const std::vector<std::string> fonts{std::string("皮肤：") + skinFontName, "衬线体", "无衬线体", "粗体",
                                             "等宽体"};
        select("界面字体", "interface-font", "interfaceFont", fonts);
        select("卡名字体", "card-name-font", "cardNameFont", fonts);
        select("点数样式", "card-value-font", "cardValueFont", fonts);
        heading("卡面");
        select("边框样式", "card-frame", "cardFrame", {"徽章", "无边框", "印刷"});
        select("数值样式", "card-ranks", "rankLayout", {"边缘数值", "经典数值"});
        toggle("显示卡名", "card-names", "showCardNames");
        toggle("显示分组", "card-groups", "showCardGroups");
        if (face.legacy())
            select("铭牌样式", "nameplate", "nameplateStyle", {"阴影带", "柔和渐变", "印在原画"});
        select("字型", "name-case", "nameCase", {"跟随皮肤", "首字母大写", "全大写"});
        if (face.legacy()) {
            heading("点数");
            slider("阴影不透明度", "rank-shadow-opacity", "rankShadowOpacity", 100, "%d%%");
            slider("阴影模糊", "rank-shadow-blur", "rankShadowBlur", 8);
            slider("阴影偏移", "rank-shadow-offset", "rankShadowOffset", 6);
            toggle("描边", "rank-stroke", "rankStroke", "皮肤");
            if (valueOf("rankStroke"))
                slider("描边粗细", "rank-stroke-weight", "rankStrokeWeight", 30, "%.1f px", true);
            else
                textBlock("跟随皮肤：1 px 描边。", width, 18, palette().muted);
            if (action("重置卡牌数值")) send("settings", "rank-finish-reset 0");
        }
        heading("卡背");
        int i = 0;
        for (const auto& back : arrayMember(state, "cardBacks")) {
            const auto pos = ImGui::GetCursorScreenPos();
            ImGui::PushID(i++);
            if (chapter("back", label(back, "name"), pos + ImVec2(72 * scale, 0),
                        ImVec2(width - 72 * scale, 85 * scale), label(state, "cardBackID") == label(back, "id"),
                        flag(back, "custom") ? "自定义" : ""))
                send("cardback", "select " + label(back, "id"));
            auto image = assets.get(label(back, "art"));
            ImGui::GetWindowDrawList()->AddImage(image.id, pos, pos + ImVec2(54, 72) * scale);
            ImGui::PopID();
        }
        if (action("刷新卡背")) send("cardback", "refresh");
        textBlock("把自定义图片放进卡背文件夹后点刷新。", width, 18);
        if (!label(s, "cardBackNotice").empty()) textBlock(label(s, "cardBackNotice"), width, 18, palette().accent);
    } else if (settingsPage == 0) {
        textBlock("对手卡面", width, 21, palette().accent);
        ImGui::Dummy(ImVec2(0, 16 * scale));
        select("版本", "match-art-treatment", "opponentTreatment", {"原色", "低饱和", "灰度", "棕褐"});
        if (valueOf("opponentTreatment") != 0) {
            select("应用于", "match-art-scope", "opponentScope", {"手牌", "牌桌", "手牌与牌桌"});
            slider("强度", "match-art-strength", "opponentStrength", 100, "%d%%");
        }
        heading("控制色");
        select("调色板", "match-palette", "controlPalette",
               {skin().player && skin().opponent ? "皮肤配色" : "Tessera", "蓝 · 琥珀", "紫 · 薄荷",
                "象牙 · 石板", "自定义"});
        if (valueOf("controlPalette") == 4) {
            const auto colour = [&](const char* title, const char* key, const char* json) {
                const auto p = ImGui::GetCursorScreenPos();
                text(p + ImVec2(0, 8) * scale, title, 18, palette().ink, width * .51f);
                const unsigned rgb = valueOf(json);
                float channels[]{float(rgb >> 16) / 255.f, float((rgb >> 8) & 255) / 255.f, float(rgb & 255) / 255.f};
                ImGui::SetCursorScreenPos(p + ImVec2(width * .56f, 0));
                ImGui::SetNextItemWidth(width * .44f);
                const bool changed = ImGui::ColorEdit3((std::string("##") + key).c_str(), channels,
                                                       ImGuiColorEditFlags_DisplayHex | ImGuiColorEditFlags_NoOptions |
                                                           ImGuiColorEditFlags_NoDragDrop);
                const int packed = (int(channels[0] * 255 + .5f) << 16) | (int(channels[1] * 255 + .5f) << 8) |
                                   int(channels[2] * 255 + .5f);
                if (changed) state["settings"][json] = packed;
                if (ImGui::IsItemDeactivatedAfterEdit())
                    send("settings", std::string(key) + " " + std::to_string(packed));
                // Ready-made colours: one press each, which is all a pad needs.
                constexpr unsigned swatches[]{0x87c4dd, 0x6fa8f5, 0x7fd6c2, 0x9fd18b, 0xb8c46a, 0xe6bc65,
                                              0xf2a25c, 0xdf9585, 0xe57fb0, 0xc9a0e8, 0xf0f0e6, 0x8c9bb0};
                const float box = 30 * scale, gap = 8 * scale;
                auto* draw = ImGui::GetWindowDrawList();
                for (int n = 0; n < int(std::size(swatches)); ++n) {
                    const auto at = p + ImVec2(n * (box + gap), 46 * scale);
                    ImGui::SetCursorScreenPos(at);
                    ImGui::PushID(n);
                    const bool pressed = ImGui::InvisibleButton((std::string(key) + "-swatch").c_str(),
                                                                ImVec2(box, box), ImGuiButtonFlags_EnableNav);
                    const bool focus = ImGui::IsItemHovered() || (ImGui::IsItemFocused() && ImGui::GetIO().NavVisible);
                    ImGui::PopID();
                    const unsigned c = swatches[n];
                    draw->AddRectFilled(at, at + ImVec2(box, box), IM_COL32(c >> 16, (c >> 8) & 255, c & 255, 255),
                                        3 * scale);
                    if (c == rgb || focus)
                        draw->AddRect(at - ImVec2(3, 3) * scale, at + ImVec2(box, box) + ImVec2(3, 3) * scale,
                                      c == rgb ? palette().ink : palette().accent, 4 * scale, 0, 2 * scale);
                    if (pressed && !pendingCommand) {
                        state["settings"][json] = int(c);
                        send("settings", std::string(key) + " " + std::to_string(c));
                    }
                }
                ImGui::SetCursorScreenPos(p + ImVec2(0, 100) * scale);
                ImGui::Dummy(ImVec2(width, 0));
            };
            colour("你", "match-player-colour", "playerColour");
            colour("对手", "match-opponent-colour", "opponentColour");
        }
        heading("卡牌边框");
        if (face.legacy() || matchAppearance.marker) {
            if (!face.legacy()) textBlock("在卡框外再加一层边框。", width, 18, palette().muted);
            select("样式", "match-border-style", "controlBorderStyle",
                   {"单线", "双线", "切角", "角括号", "原版 Tessera"});
            const bool skinWidth = valueOf("controlBorderWidth") == 12;
            slider("粗细", "match-border-width", "controlBorderWidth", 40, "%.1f px", true, skinWidth);
            if (skinWidth) textBlock("跟随皮肤：1.8 px 边框。", width, 18, palette().muted);
            slider("不透明度", "match-border-opacity", "controlBorderOpacity", 100, "%d%%");
        } else
            textBlock("卡框使用你的控制色。开启控制标记可再加一层边框。", width, 18, palette().muted);
        toggle("控制标记", "match-control-marker", "controlMarker");
        if (valueOf("controlMarker")) textBlock("实心为你的，描边为对手的。", width, 18);
        ImGui::Dummy(ImVec2(0, 16 * scale));
        if (action("重置对局外观")) send("settings", "match-appearance-reset 0");
    } else if (settingsPage == 1) {
        for (const auto& field : ttcg::HotkeyBindings::fields) {
            const auto p = ImGui::GetCursorScreenPos();
            const std::string id(field.name), name(field.label);
            text(p + ImVec2(0, 8) * scale, name, 21, palette().ink, width * .4f);
            const auto binding = member(s, id.c_str());
            unsigned code = binding.is_number_integer() ? binding.get<unsigned>() : 0;
            const bool capture = label(s, "capturing") == id;
            if (button(id.c_str(), capture ? "按下按键…" : keyLabel(code).c_str(), p + ImVec2(width * .43f, 0),
                       ImVec2(width * .39f, 40 * scale), capture, !pendingCommand))
                send("binding", std::string(capture ? "cancel" : "begin " + id));
            if (button((id + "-clear").c_str(), "×", p + ImVec2(width - 115 * scale, 0), ImVec2(35, 40) * scale, false,
                       !capturing()))
                send("binding", "unbind " + id);
            if (button((id + "-reset").c_str(), "重置", p + ImVec2(width - 72 * scale, 0), ImVec2(72, 40) * scale,
                       false, !capturing()))
                send("binding", "reset " + id);
            ImGui::SetCursorScreenPos(p + ImVec2(0, 79) * scale);
            ImGui::Dummy(ImVec2(width, 0));
        }
        textBlock("左右 Shift、Ctrl、Alt 可与按键组合。Esc 取消。", width, 18);
        if (!label(s, "notice").empty()) textBlock(label(s, "notice"), width, 18, palette().accent);
        heading("Tessera 显示");
        textBlock(
            "先点一张卡，再点牌桌格子出牌，也可以直接把卡拖过去。编辑牌组时，可把卡拖进槽位，或双击、按回车放入。右键点卡或按 I 检视。Q、E 切换页签。Tab 与方向键移动焦点，回车选中，Esc 返回。",
            width, 18);
        ImGui::Dummy(ImVec2(0, 10 * scale));
        textBlock(
            "手柄上，十字键或左摇杆移动焦点，A 选中。右摇杆移动指针，A 点击指针下的东西。Y 检视卡牌，X 加入牌组，LB、RB 切换页签，B 返回。",
            width, 18);
    } else if (settingsPage == 2) {
        const auto& collectionState = member(state, "collection");
        const auto missing = join(arrayMember(collectionState, "missingSets"), ", ");
        if (!missing.empty()) {
            textBlock("不可用的卡", width, 21, palette().accent);
            ImGui::Dummy(ImVec2(0, 16 * scale));
            textBlock("缺少扩展：" + missing +
                          "。重新安装可恢复卡牌，或移除这些卡在本角色上的存档副本。",
                      width, 18);
            const auto blocked = label(collectionState, "recoveryBlocked");
            ImGui::BeginDisabled(pendingCommand || capturing() || !member(state, "interruptedMatch").empty() ||
                                 !blocked.empty());
            if (action("移除不可用的卡"))
                confirm(
                    "移除不可用的卡？",
                    "会从本角色移除不可用的卡及其发现记录，并清空它们占用的牌组槽位。已安装的卡与游戏进度保留。重新安装扩展也不会再还原这些副本。如果日后可能想找回，请保留较旧的存档。",
                    "collection-recovery", "remove-missing confirm", "移除卡牌");
            ImGui::EndDisabled();
            if (!blocked.empty()) textBlock(blocked, width, 18, palette().accent);
            heading("玩法");
        }
        toggle("启用 Tessera 对局", "games", "gamesEnabled");
        textBlock("关闭 Tessera 对局。相册仍可查看。", width, 18);
        toggle("出牌前显示占地", "capture-preview", "capturePreview");
        textBlock("选好一张卡后，牌桌会在每个格子上标出它能占下的卡。", width, 18);
        if (const auto packs = label(s, "merchantPacks"); !packs.empty()) {
            heading("商人卡包");
            textBlock(packs, width, 18, palette().accent);
        }
        heading("模组对手");
        select("居民", "residents", "residents", {"禁用", "部分居民", "全部符合条件的居民"});
        toggle("仅原版语音", "vanilla-voices", "vanillaVoicesOnly");
        textBlock("自制对手可以使用各自的语音。", width, 18);
        heading("添加玩家");
        textBlock(label(s, "residentCandidate", "先看向一名符合条件的 NPC，再打开相册。"), width, 18);
        if (button("add-resident", "添加玩家", ImGui::GetCursorScreenPos(), ImVec2(200, 40) * scale, false,
                   flag(s, "canAddResident") && !pendingCommand))
            send("npc-add");
        heading("世界暂停");
        toggle("相册中暂停", "pause-album", "pauseWorldInAlbum");
        heading("实验性功能");
        textBlock("未暂停的对局中，对手移动或进入场景时会被打断。", width, 18);
        toggle("商店中暂停", "pause-shop", "pauseWorldInShops");
        toggle("对局准备中暂停", "pause-setup", "pauseWorldInMatchSetup");
        toggle("对局中暂停", "pause-match", "pauseWorldDuringMatches");
    } else {
        const auto devToggle = [&](const char* name, const char* command, const char* json) {
            const auto p = ImGui::GetCursorScreenPos();
            const bool value = flag(dev, json);
            text(p + ImVec2(0, 8) * scale, name, 18, palette().ink);
            if (button(command, value ? "开" : "关", p + ImVec2(width - 68 * scale, 0), ImVec2(68, 36) * scale, value))
                send("development", std::string("set ") + command + " " + (value ? "0" : "1"));
            ImGui::SetCursorScreenPos(p + ImVec2(0, 58) * scale);
            ImGui::Dummy(ImVec2(width, 0));
        };
        heading("收藏");
        if (action("添加全部卡牌")) send("development", "cards");
        ImGui::SameLine();
        if (action("添加全部闪卡")) send("development", "foils");
        const auto blocked = label(member(state, "collection"), "recoveryBlocked");
        ImGui::Dummy(ImVec2(0, 12 * scale));
        ImGui::BeginDisabled(pendingCommand || capturing() || !member(state, "interruptedMatch").empty() ||
                             !blocked.empty());
        if (action("清空收藏"))
            confirm(
                "清空你的收藏？",
                "会删除本角色所有卡与闪卡（含不可用的卡），清除发现记录并清空全部牌组。相册、金币与游戏进度保留。自动发卡与“揭示全部卡牌”会对所有角色关闭。删除后只能靠较早的存档找回。",
                "collection-recovery", "clear confirm", "清空收藏");
        ImGui::EndDisabled();
        textBlock(blocked.empty() ? "从空相册重新开始收集。游戏进度保留。" : blocked,
                  width, 18);
        ImGui::Dummy(ImVec2(0, 18 * scale));
        const auto resetBlocked = label(member(state, "collection"), "resetBlocked", blocked);
        ImGui::BeginDisabled(pendingCommand || capturing() || !member(state, "interruptedMatch").empty() ||
                             !resetBlocked.empty());
        if (action("重置 Tessera"))
            confirm(
                "为本角色重置 Tessera？",
                "会清空收藏、牌组、战绩、声望、锦标赛与地区规则改动，重置 NPC 与商人收藏，取消换卡，并移除携带的卡包与旧邀请函。相册保留，并发放八张起始卡与一套起始牌组。金币与界面偏好不变，开发者覆盖项对所有角色恢复默认。只能靠较早的存档撤销。",
                "collection-recovery", "reset confirm", "重置 Tessera");
        ImGui::EndDisabled();
        textBlock(resetBlocked.empty()
                      ? "用相册与起始卡重新开始 Tessera。所有 Tessera 进度会被清除。"
                      : resetBlocked,
                  width, 18);
        heading("锦标赛");
        devToggle("常规锦标赛", "RegularTournaments", "RegularTournaments");
        devToggle("邀请赛", "InvitationalTournaments", "InvitationalTournaments");
        devToggle("大师赛", "MastersTournaments", "MastersTournaments");
        std::vector<std::string> holds;
        for (const auto& h : arrayMember(member(state, "collection"), "regions")) holds.push_back(label(h, "name"));
        int hold = std::clamp(number(dev, "TournamentHold", 1) - 1, 0, 9);
        if (!holds.empty() && combo("dev-hold", "信使领地", hold, holds, width))
            send("development", "set TournamentHold " + std::to_string(hold + 1));
        heading("玩家");
        devToggle("显示全部玩家", "ShowAllPlayers", "ShowAllPlayers");
        devToggle("解锁全部玩家", "UnlockAllPlayers", "UnlockAllPlayers");
        devToggle("揭示全部卡牌", "RevealAllCards", "RevealAllCards");
        devToggle("允许任意 NPC", "AllowAnyNPC", "AllowAnyNPC");
        devToggle("记录战绩", "RecordResults", "RecordResults");
        heading("对局覆盖");
        int rules = number(dev, "Rules", -1);
        bool inherit = rules < 0;
        if (ImGui::Checkbox("使用地区规则", &inherit))
            send("development", "set Rules " + std::to_string(inherit ? -1 : 0));
        if (!inherit)
            for (auto [bit, name] : ruleNames) {
                bool value = (rules & bit) != 0;
                const auto shown = zhRuleName(name);
                if (ImGui::Checkbox((shown + "##" + name).c_str(), &value)) {
                    const auto proposed =
                        value ? ttcg::withSpreadRule(rules, bit) : ttcg::withoutCultureRule(rules, bit);
                    send("development", "set Rules " + std::to_string(proposed));
                }
            }
        int trade = number(dev, "TradeRule", -1) + 1;
        if (combo("dev-trade", "交易规则", trade, {"默认", "无", "一张", "分差", "直接", "全部"}, width))
            send("development", "set TradeRule " + std::to_string(trade - 1));
        int skill = number(dev, "OpponentSkill", -1) + 1;
        if (combo("dev-skill", "对手水平", skill,
                  {"默认", "新手", "学徒", "普通", "熟练", "专家", "大师"}, width))
            send("development", "set OpponentSkill " + std::to_string(skill - 1));
        std::vector<int> wagers{-1};
        std::vector<std::string> wagerLabels{"默认"};
        int wager = 0;
        for (int amount : ttcg::wagerAmounts) {
            wagers.push_back(amount);
            wagerLabels.push_back(std::to_string(amount));
            if (amount == number(dev, "Wager", -1)) wager = int(wagers.size()) - 1;
        }
        if (combo("dev-wager", "赌注", wager, wagerLabels, width))
            send("development", "set Wager " + std::to_string(wagers[wager]));
        int lesson = number(dev, "RuleLesson");
        if (combo("dev-lesson", "规则教学", lesson, {"关闭", "三明牌", "军团", "减损", "亲和"},
                  width))
            send("development", "set RuleLesson " + std::to_string(lesson));
        if (!label(dev, "notice").empty()) textBlock(label(dev, "notice"), width, 18, palette().accent);
    }
    if (settingsPage == 0 && !label(s, "interfaceNotice").empty())
        textBlock(label(s, "interfaceNotice"), width, 18, palette().accent);
    ImGui::Dummy(ImVec2(1, 30 * scale));
    ImGui::EndChild();
    applySettings();
    if (settingsPage == 0 && appearancePage == 1) {
        matchAppearancePreview(at + ImVec2(size.x - preview, 0), ImVec2(preview, size.y));
        return;
    }
    if (settingsPage == 0 && !collection.cards.empty()) {
        auto p = at + ImVec2(size.x - preview, 58 * scale);
        line(p - ImVec2(gap / 2, 0), p + ImVec2(-gap / 2, size.y - 58 * scale));
        const float w = preview - 12 * scale, h = w / .75f;
        if (previewBack) {
            auto image = assets.get(customBack);
            ImGui::GetWindowDrawList()->AddImage(image.id, p, p + ImVec2(w, h));
        } else
            displayCard(selected, p, ImVec2(w, h), false, -1, MaterialSlot::single, 0, true);
        auto buttons = p + ImVec2(0, h + 22 * scale);
        const auto step = [&](int dir) {
            for (int i = 0; i < int(collection.cards.size()); ++i) {
                selected = (selected + dir + int(collection.cards.size())) % int(collection.cards.size());
                if (collection.cards[selected].known[0] || collection.cards[selected].owned[0]) break;
            }
        };
        if (button("previous-preview", "←", buttons, ImVec2(35, 36) * scale)) step(-1);
        if (button("next-preview", "→", buttons + ImVec2(w - 35 * scale, 0), ImVec2(35, 36) * scale)) step(1);
        text(buttons + ImVec2(w / 2, 0), ttcg::zhCardName(collection.cards[selected].name), 18, palette().muted, w - 90 * scale, true);
        if (button("turn-preview", previewBack ? "看正面" : "看背面",
                   buttons + ImVec2((w - 145 * scale) / 2, 58 * scale), ImVec2(145, 38) * scale))
            previewBack = !previewBack;
    }
}
}
