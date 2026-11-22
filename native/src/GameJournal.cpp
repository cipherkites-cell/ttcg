#include "GameApp.h"
#include "Announcements.h"
namespace preview {
void GameApp::journal(ImVec2 at, ImVec2 size) {
    const auto& c = member(state, "collection");
    const auto& regions = arrayMember(c, "regions");
    if (journalHold < 0) journalHold = number(c, "holdID");
    const float nav = AlbumLayout::navigation * scale, gap = AlbumLayout::gap * scale;
    line(at + ImVec2(nav + gap / 2, 0), at + ImVec2(nav + gap / 2, size.y));
    ImGui::SetCursorScreenPos(at);
    ImGui::BeginChild("holds", ImVec2(nav, size.y), ImGuiChildFlags_NavFlattened);
    const float navWidth = nav - 20 * scale;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 6 * scale));
    if (chapter("all", "全部领地", ImGui::GetCursorScreenPos(), ImVec2(navWidth, 43 * scale), journalHold == 0)) {
        journalHold = 0;
        journalPlayer = -1;
        journalQuery[0] = 0;
    }
    for (const auto& hold : regions) {
        int id = number(hold, "id");
        if (chapter("hold" + std::to_string(id), ttcg::zhHoldText(label(hold, "name")), ImGui::GetCursorScreenPos(),
                    ImVec2(navWidth, 43 * scale), journalHold == id)) {
            journalHold = id;
            journalPlayer = -1;
            journalQuery[0] = 0;
        }
    }
    ImGui::PopStyleVar();
    ImGui::EndChild();
    auto mid = at + ImVec2(nav + gap, 0);
    float width = size.x - nav - gap;
    const Json* chosen = nullptr;
    for (const auto& h : regions)
        if (number(h, "id") == journalHold) chosen = &h;
    text(mid,
         chosen              ? ttcg::zhHoldText(label(*chosen, "name"))
         : journalHold == 11 ? "旅行者"
                             : "全部领地",
         32, palette().accent);
    float y = mid.y + 53 * scale;
    if (chosen && !flag(*chosen, "travellers") && !flag(*chosen, "unaffiliated")) {
        text(ImVec2(mid.x, y),
             ttcg::zhReputationText(label(*chosen, "reputationTitle")) + " · 胜场：" +
                 std::to_string(number(*chosen, "wins")),
             21, palette().muted);
        y += 38 * scale;
        const auto& changes = arrayMember(*chosen, "ruleChanges");
        y += ruleChips(ImVec2(mid.x, y), width, number(*chosen, "rules"), number(*chosen, "trade"), false, true,
                       &changes) +
             10 * scale;
    }
    line(ImVec2(mid.x, y), ImVec2(mid.x + width, y));
    y += 15 * scale;
    ImGui::SetCursorScreenPos(ImVec2(mid.x, y));
    ImGui::SetNextItemWidth(width - 205 * scale);
    ImGui::InputTextWithHint("##players", "查找玩家", journalQuery, sizeof(journalQuery));
    ImGui::SetCursorScreenPos(ImVec2(mid.x + width - 190 * scale, y));
    select("##player-filter", journalFilter, {"全部玩家", "已对战", "未对战"}, 190 * scale);
    y += 60 * scale;
    std::vector<int> rows;
    const auto& roster = arrayMember(c, "roster");
    for (int i = 0; i < int(roster.size()); ++i) {
        const auto& p = roster[i];
        if (journalHold == 11 ? !flag(p, "traveller") : journalHold && number(p, "holdID") != journalHold) continue;
        const bool played =
            !flag(p, "friendlyOnly") && number(p, "wins") + number(p, "losses") + number(p, "draws") > 0;
        if (journalFilter == 1 && !played || journalFilter == 2 && played) continue;
        if (journalQuery[0] &&
            lower(label(p, "name") + " " + label(p, "hold") + " " + label(p, "skill")).find(lower(journalQuery)) ==
                std::string::npos)
            continue;
        rows.push_back(i);
    }
    std::sort(rows.begin(), rows.end(),
              [&](int a, int b) { return label(roster[a], "name") < label(roster[b], "name"); });
    if (std::find(rows.begin(), rows.end(), journalPlayer) == rows.end())
        journalPlayer = rows.empty() ? -1 : rows.front();
    const float detailWidth = journalPlayer >= 0 ? 260 * scale : 0,
                listWidth = width - (detailWidth ? detailWidth + gap : 0);
    textRight(ImVec2(mid.x + width - 12 * scale, y - 7 * scale), std::to_string(rows.size()) + " 名玩家", 15,
              palette().muted);
    y += 24 * scale;
    ImGui::SetCursorScreenPos(ImVec2(mid.x, y));
    ImGui::BeginChild("players", ImVec2(listWidth, at.y + size.y - y), ImGuiChildFlags_NavFlattened);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 8 * scale));
    const float rowHeight = 68 * scale, rowWidth = ImGui::GetContentRegionAvail().x - 12 * scale;
    ImGuiListClipper clip;
    clip.Begin(int(rows.size()), rowHeight + 8 * scale);
    while (clip.Step())
        for (int row = clip.DisplayStart; row < clip.DisplayEnd; ++row) {
            const int i = rows[row];
            const auto& p = roster[i];
            const auto pos = ImGui::GetCursorScreenPos();
            const bool friendly = flag(p, "friendlyOnly");
            if (chapter("player" + std::to_string(i), label(p, "name"), pos, ImVec2(rowWidth, rowHeight),
                        journalPlayer == i, ttcg::zhSkillText(label(p, "skill")), 180 * scale))
                journalPlayer = i;
            std::string record = friendly               ? "练习"
                                 : !flag(p, "unlocked") ? "未解锁"
                                 : number(p, "wins") + number(p, "losses") + number(p, "draws")
                                     ? std::to_string(number(p, "wins")) + " 胜 · " +
                                           std::to_string(number(p, "losses")) + " 负 · " +
                                           std::to_string(number(p, "draws")) + " 平"
                                     : "未对战";
            text(pos + ImVec2(rowWidth - 180 * scale, 14 * scale), record, 15, palette().muted, 168 * scale);
        }
    if (rows.empty())
        text(ImGui::GetCursorScreenPos() + ImVec2(listWidth / 2, 48 * scale), "没有已知的玩家", 21, palette().muted, 0,
             true);
    ImGui::PopStyleVar();
    ImGui::EndChild();
    if (journalPlayer >= 0 && journalPlayer < int(roster.size())) {
        const auto& p = roster[journalPlayer];
        const auto detail = ImVec2(mid.x + width - detailWidth, y);
        line(detail - ImVec2(gap / 2, 0), detail + ImVec2(-gap / 2, at.y + size.y - y));
        // Give flowing text a real column origin and clip rectangle. A cursor
        // offset in the root window is lost at the next ImGui layout item.
        ImGui::SetCursorScreenPos(detail);
        ImGui::BeginChild("player-details", ImVec2(detailWidth, at.y + size.y - y), ImGuiChildFlags_NavFlattened);
        const auto top = ImGui::GetCursorScreenPos();
        text(top, label(p, "name"), 26, palette().accent, detailWidth);
        float ry =
            top.y + font()->CalcTextSizeA(26 * scale, 1e5f, detailWidth, label(p, "name").c_str()).y + 12 * scale;
        skinRule(ImVec2(top.x, ry), ImVec2(top.x + detailWidth, ry), .7f);
        ry += 8 * scale;
        // The same labelled rows the card details use.
        const int wins = number(p, "wins"), losses = number(p, "losses"), draws = number(p, "draws"),
                  games = wins + losses + draws;
        const bool friendly = flag(p, "friendlyOnly");
        std::vector<std::pair<const char*, std::string>> facts{
            {"出现地点", label(p, "location")},
            {"领地", flag(p, "traveller") ? "旅行者" : ttcg::zhHoldText(label(p, "hold"))},
            {"水平", ttcg::zhSkillText(label(p, "skill"))},
            {"对局", friendly              ? "仅练习"
                     : flag(p, "unlocked") ? "可对战"
                                           : "尚未就绪"}};
        if (!friendly && p.contains("rules")) {
            // What a game with them is played for. M'aiq's rules change with every game.
            const auto rules = rulesLabel(unsigned(number(p, "rules")));
            facts.push_back({flag(p, "ownRules") ? "自有规则" : "规则", flag(p, "traveller") && flag(p, "ownRules")
                                                                            ? "每局不同"
                                                                        : rules.empty() ? "暗牌"
                                                                                        : rules});
            facts.push_back({"交易", zhTradeName(number(p, "trade"))});
            if (number(p, "wager") > 0)
                facts.push_back({"赌注", "最高 " + std::to_string(number(p, "wager")) + " 金币"});
        }
        if (!friendly) {
            facts.push_back({"已对战", std::to_string(games)});
            facts.push_back({"胜", std::to_string(wins)});
            facts.push_back({"负", std::to_string(losses)});
            facts.push_back({"平", std::to_string(draws)});
            if (games) facts.push_back({"胜率", std::to_string(wins * 100 / games) + "%"});
        }
        for (const auto& [name, value] : facts) {
            if (value.empty()) continue;
            textCaps(ImVec2(top.x, ry + 9 * scale), name, 13.5f, palette().muted, 1.2f);
            const float room = detailWidth - capsWidth(name, 13.5f, 1.2f) - 14 * scale,
                        natural = font()->CalcTextSizeA(18 * scale, 1e5f, 0, value.c_str()).x;
            const float pixels = std::clamp(17.f * room / std::max(1.f, natural), 15.f, 17.f);
            textRight(ImVec2(top.x + detailWidth - 2 * scale, ry + 5 * scale), fitted(value, pixels, room), pixels,
                      palette().ink);
            ry += 31 * scale;
            ImGui::GetWindowDrawList()->AddLine(ImVec2(top.x, ry - 1), ImVec2(top.x + detailWidth, ry - 1),
                                                fade(palette().line, .55f));
        }
        ImGui::SetCursorScreenPos(ImVec2(top.x, ry));
        ImGui::Dummy(ImVec2(1, 1));
        ImGui::EndChild();
    }
}
}
//（注：内容由AI生成）
