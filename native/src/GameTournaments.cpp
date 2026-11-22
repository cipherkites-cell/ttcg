#include "GameApp.h"
#include <bit>
#include "Announcements.h"
namespace preview {
void GameApp::tournaments(ImVec2 at, ImVec2 size) {
    const auto& c = member(state, "collection");
    const auto& data = member(c, "tournaments");
    const auto& all = arrayMember(data, "events");
    const float nav = AlbumLayout::navigation * scale, gap = AlbumLayout::gap * scale;
    line(at + ImVec2(nav + gap / 2, 0), at + ImVec2(nav + gap / 2, size.y));
    if (button("available", "可参加", at, ImVec2(103, 35) * scale, !history)) {
        history = false;
        eventID = 0;
    }
    if (button("history", "历史", at + ImVec2(111, 0) * scale, ImVec2(85, 35) * scale, history)) {
        history = true;
        eventID = 0;
    }
    std::vector<const Json*> events;
    for (const auto& e : all)
        if (history ? flag(e, "joined") && number(e, "champion") != 0
                    : !number(e, "champion") && number(data, "hour") < number(e, "ends"))
            events.push_back(&e);
    const auto priority = [&](const Json* e) {
        return flag(*e, "joined") && !flag(*e, "eliminated")       ? 0
               : flag(*e, "canEnter")                              ? 1
               : number(*e, "hold") == number(data, "currentHold") ? 2
                                                                   : 3;
    };
    std::sort(events.begin(), events.end(), [&](const auto* a, const auto* b) {
        if (history) return number(*a, "ends") > number(*b, "ends");
        if (priority(a) != priority(b)) return priority(a) < priority(b);
        return number(*a, "ends") < number(*b, "ends");
    });
    if (std::none_of(events.begin(), events.end(), [&](const auto* e) { return number(*e, "id") == int(eventID); }))
        eventID = events.empty() ? 0 : number(*events.front(), "id");
    ImGui::SetCursorScreenPos(at + ImVec2(0, 54) * scale);
    ImGui::BeginChild("tournament-list", ImVec2(nav, size.y - 54 * scale), ImGuiChildFlags_NavFlattened);
    for (auto* e : events) {
        const int id = number(*e, "id");
        const bool won = number(*e, "champion") == 20;
        const bool unseen = flag(*e, "unseen") && !seenEvents.contains(id);
        if (chapter("event" + std::to_string(id), label(*e, "name"), ImGui::GetCursorScreenPos(),
                    ImVec2(nav - 20 * scale, 75 * scale), id == int(eventID), label(*e, "circuitName"), 0,
                    won      ? 1
                    : unseen ? 2
                             : 0))
            eventID = id;
        ImGui::Dummy(ImVec2(0, 4 * scale));
    }
    ImGui::EndChild();
    const auto origin = at + ImVec2(nav + gap, 0);
    const float width = size.x - nav - gap;
    const auto* current = tournament(eventID);
    if (!current) {
        text(origin, history ? "没有锦标赛记录" : "和旅店老板交谈即可查找并报名锦标赛。", 21, palette().muted, width);
        return;
    }
    const auto& e = *current;
    if (flag(e, "unseen") && !seenEvents.contains(eventID)) {
        seenEvents.insert(eventID);
        send("tournament", "seen " + std::to_string(eventID), false);
    }
    text(origin, label(e, "name") + " · " + label(e, "circuitName"), 32, palette().accent, width - 170 * scale);
    const auto champion = number(e, "champion");
    text(origin + ImVec2(width - 170 * scale, 14 * scale),
         champion                ? (champion == 20          ? "冠军"
                                    : flag(e, "eliminated") ? "被淘汰"
                                                            : "已结束")
         : flag(e, "eliminated") ? "已退赛"
         : flag(e, "joined")     ? "进行中"
                                 : "报名中",
         18, palette().accent, 170 * scale);
    text(origin + ImVec2(0, 44) * scale, label(e, "venue") + " · " + label(e, "host"), 18, palette().muted);
    line(origin + ImVec2(0, 82) * scale, origin + ImVec2(width, 82 * scale));
    ruleChips(origin + ImVec2(0, 91) * scale, width, number(e, "rules"), -1, false, true);
    text(origin + ImVec2(0, 135) * scale, (champion ? "已结束 " : "截止 ") + label(e, "endsLabel"), 15,
         palette().muted);
    const float prizes = 205 * scale, boardW = width - prizes - gap, top = 185 * scale,
                availableH = size.y - top - 90 * scale, colGap = 15 * scale, colW = (boardW - 2 * colGap) / 3;
    for (int col = 0; col < 3; ++col) {
        const char* title = col == 0 ? "四分之一决赛" : col == 1 ? "半决赛" : "决赛";
        text(origin + ImVec2(col * (colW + colGap), top - 20 * scale), title, 18, palette().muted);
        const int start = col == 0 ? 8 : col == 1 ? 12 : 14, count = col == 0 ? 4 : col == 1 ? 2 : 1;
        const float section = availableH / count, pairH = std::min(86 * scale, section - 12 * scale);
        for (int pair = 0; pair < count; ++pair) {
            const int node = start + pair;
            const auto pos = origin + ImVec2(col * (colW + colGap), top + (pair + .5f) * section - pairH / 2);
            auto* d = ImGui::GetWindowDrawList();
            d->AddRectFilled(pos, pos + ImVec2(colW, pairH), IM_COL32(0, 0, 0, 40));
            d->AddRect(pos, pos + ImVec2(colW, pairH), palette().line);
            if (col < 2) {
                // The winner's path to the next round.
                const float next = availableH / (count / 2);
                const ImVec2 from = pos + ImVec2(colW, pairH / 2),
                             to = origin + ImVec2((col + 1) * (colW + colGap), top + (pair / 2 + .5f) * next);
                const ImVec2 path[]{from, ImVec2(from.x + colGap * .5f, from.y), ImVec2(from.x + colGap * .5f, to.y),
                                    to};
                d->AddPolyline(path, 4, fade(palette().muted, .6f), 0, std::max(1.f, scale));
            }
            line(pos + ImVec2(0, pairH / 2), pos + ImVec2(colW, pairH / 2));
            for (int side = 0; side < 2; ++side) {
                const int slot = (node - 8) * 2 + side;
                const auto& bracket = member(e, "bracket");
                if (!bracket.is_array() || slot >= int(bracket.size())) continue;
                const auto& player = bracket[slot];
                const int wins = itemNumber(member(e, "wins")[node - 8], side, 0);
                const auto row = pos + ImVec2(0, side * pairH / 2);
                if (number(player, "id") == 20)
                    d->AddRectFilled(row, row + ImVec2(colW, pairH / 2), (palette().selection & 0xffffff) | 0x55000000);
                text(row + ImVec2(10, 10) * scale, flag(player, "reserved") ? "已预留" : label(player, "name"), 15,
                     wins == 2 ? palette().accent : palette().ink, colW - 40 * scale);
                if (number(player, "id"))
                    text(row + ImVec2(colW - 24 * scale, 10 * scale), std::to_string(wins), 18, palette().muted);
            }
        }
    }
    const auto prizeAt = origin + ImVec2(width - prizes, top);
    line(prizeAt - ImVec2(gap / 2, 15 * scale), prizeAt + ImVec2(-gap / 2, availableH));
    textCaps(prizeAt, "奖金", 13.5f, palette().muted, 1.6f);
    text(prizeAt + ImVec2(0, 26) * scale, std::to_string(number(e, "purse")) + " 金币", 26, skin().value);
    text(prizeAt + ImVec2(0, 62) * scale, label(e, "prize"), 18, palette().ink, prizes);
    if (arrayMember(e, "prizeCards").empty())
        if (const auto pack = assets.pack(1); pack.id) {
            // The pack itself, so the prize is something to look at.
            const float h = std::min(190 * scale, availableH - 190 * scale), w = h * pack.width / pack.height;
            if (h > 60 * scale)
                ImGui::GetWindowDrawList()->AddImage(pack.id, prizeAt + ImVec2((prizes - w) * .5f, 112 * scale),
                                                     prizeAt + ImVec2((prizes + w) * .5f, 112 * scale + h));
        }
    int pindex = 0;
    const auto& prizesList = arrayMember(e, "prizeCards");
    for (const auto& id : prizesList) {
        const float cardW = prizesList.size() > 1 ? (prizes - 10 * scale) / 2 : prizes;
        const auto pos = prizeAt + ImVec2((pindex % 2) * (cardW + 10 * scale),
                                          103 * scale + (pindex / 2) * (cardW / .75f + 15 * scale));
        const ImVec2 extent(cardW, cardW / .75f);
        ImGui::SetCursorScreenPos(pos);
        if (ImGui::InvisibleButton(("prize" + std::to_string(pindex)).c_str(), extent, ImGuiButtonFlags_EnableNav))
            inspectCard(id.get<int>());
        displayCard(id.get<int>(), pos, extent, false, -1, MaterialSlot::eventPrizes + pindex);
        ++pindex;
    }
    if (number(e, "fee") && !flag(e, "joined"))
        text(prizeAt + ImVec2(0, availableH - 20 * scale), "报名费：" + std::to_string(number(e, "fee")) + " 金币", 15,
             palette().muted);
    const auto footer = origin + ImVec2(0, size.y - 63 * scale);
    line(footer, footer + ImVec2(width, 0));
    if (champion) return;
    const bool joined = flag(e, "joined"), play = joined && !flag(e, "eliminated"), registration = !joined;
    const auto& interrupted = member(state, "interruptedMatch");
    const bool hasInterrupted = !interrupted.is_null() && !interrupted.empty(),
               same = hasInterrupted && number(interrupted, "tournament") == int(eventID);
    std::string reason;
    int playable = 0;
    for (const auto& card : collection.cards) playable += std::min(card.unique ? 1 : 2, card.owned[0] + card.owned[1]);
    if (!flag(member(state, "settings"), "gamesEnabled", true))
        reason = "在设置里开启 Tessera 对局后才能游玩。";
    else if (hasInterrupted)
        reason = same ? flag(interrupted, "canResume") ? "" : "回到" + label(e, "venue") + "继续对局。"
                      : "请先打完被打断的对局。";
    else if (registration && flag(data, "activeEntry"))
        reason = "请先完成当前的锦标赛。";
    else if (registration && !flag(e, "canEnter"))
        reason = "去" + label(e, "venue") + "找" + label(e, "host") + "报名。";
    else if (!flag(e, "atVenue"))
        reason = "回到" + label(e, "venue") + "才能参赛。";
    else if (registration && number(c, "gold") < number(e, "fee"))
        reason = "金币不足以报名。";
    else if (registration && playable < 5)
        reason = "需要五张可用卡牌才能报名。";
    text(footer + ImVec2(0, play && !hasInterrupted ? -28 : 23) * scale, reason, 15, palette().muted,
         width - 250 * scale);
    if (play && !hasInterrupted) {
        if (button("tournament-deck", "编辑牌组", footer + ImVec2(0, 20) * scale, ImVec2(110, 36) * scale)) {
            tab = 1;
            screen = 3;
        }
        if (button("withdraw", "退赛", footer + ImVec2(120, 20) * scale, ImVec2(110, 36) * scale))
            confirm("要退出锦标赛吗？", number(e, "fee") ? "报名费不会退还。" : "本次锦标赛之旅将就此结束。",
                    "tournament", "withdraw " + std::to_string(eventID), "退赛");
    }
    if (registration || play || same) {
        const auto action = same                      ? flag(interrupted, "finished") ? "领取奖励" : "继续对局"
                            : registration            ? "报名参赛"
                            : number(e, "round") == 1 ? "进行四分之一决赛"
                            : number(e, "round") == 2 ? "进行半决赛"
                                                      : "进行决赛";
        if (button("tournament-action", action, footer + ImVec2(width - 215 * scale, 17 * scale),
                   ImVec2(215, 40) * scale, true, reason.empty() && !pendingCommand)) {
            if (same)
                send("interrupted", "resume");
            else if (registration)
                confirm(number(e, "fee") ? "以 " + std::to_string(number(e, "fee")) + " 金币报名？"
                                         : "报名参加锦标赛？",
                        "", "tournament", "enter " + std::to_string(eventID), "报名");
            else
                send("tournament", "round " + std::to_string(eventID));
        }
    }
}
void GameApp::tournamentOutcome() {
    if (!sourceReady || flag(state, "settling") || !member(state, "interruptedMatch").is_null()) return;
    const auto& c = member(state, "collection");
    const Json* current = nullptr;
    if (label(state, "phase") == "result") current = tournament(number(c, "tournamentID"));
    if (!current)
        for (const auto& e : arrayMember(member(c, "tournaments"), "events"))
            if (number(e, "announcement")) {
                current = &e;
                break;
            }
    if (!current || inspecting || showRuleReference || !confirmationTitle.empty()) return;
    const auto& e = *current;
    eventID = number(e, "id");
    if (!ImGui::IsPopupOpen("Tournament result")) ImGui::OpenPopup("Tournament result");
    const float width = 850 * scale;
    // Only prize cards and foil upgrades need the tall dialog.
    const bool tall = !arrayMember(e, "prizeCards").empty() || number(e, "foilUpgrades");
    ImGui::SetNextWindowPos(ImGui::GetIO().DisplaySize * .5f, ImGuiCond_Always, ImVec2(.5f, .5f));
    ImGui::SetNextWindowSize(
        ImVec2(width, std::min((tall ? 775 : 400) * scale, ImGui::GetIO().DisplaySize.y - 45 * scale)));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30, 28) * scale);
    if (ImGui::BeginPopupModal("Tournament result", nullptr, ImGuiWindowFlags_NoDecoration | popupFlags())) {
        popupSurface();
        const bool champion = number(e, "champion") == 20, over = number(e, "champion") || flag(e, "eliminated");
        const int announce = number(e, "announcement"), upgrades = number(e, "foilUpgrades");
        const auto outcome = itemNumber(member(state, "score"), 0) == itemNumber(member(state, "score"), 1) ? "平局"
                             : itemNumber(member(state, "score"), 0) > 5 ? "对局获胜"
                                                                         : "对局失利";
        const auto title = champion                ? "锦标赛夺冠"
                           : announce == 1         ? "晋级半决赛"
                           : announce == 2         ? "晋级决赛"
                           : flag(e, "eliminated") ? "锦标赛结束"
                                                   : outcome;
        // A centred heading in the outcome's own colour, between two rules.
        const bool lost = flag(e, "eliminated") || std::string_view(title) == "对局失利";
        const ImU32 tone = champion ? goldInk() : lost ? skin().notice : skin().value;
        const float middle = ImGui::GetWindowPos().x + width * .5f;
        const auto centred = [&](const std::string& value, float pixels, ImU32 colour, float gap) {
            const auto at = ImGui::GetCursorScreenPos();
            text(ImVec2(middle, at.y), value, pixels, colour, 0, true);
            ImGui::Dummy(ImVec2(1, (pixels * 1.3f + gap) * scale));
        };
        {
            const auto at = ImGui::GetCursorScreenPos();
            const float half = capsWidth(title, 30, 4) * .5f + 26 * scale, y = at.y + 22 * scale;
            textCaps(ImVec2(middle, at.y + 2 * scale), title, 30, tone, 4, true);
            const float reach = std::max(0.f, width * .5f - 40 * scale - half);
            gradientLine(ImGui::GetWindowDrawList(), ImVec2(middle - half - reach, y), reach, fade(tone, .7f));
            gradientLine(ImGui::GetWindowDrawList(), ImVec2(middle + half, y), reach, fade(tone, .7f));
            ImGui::Dummy(ImVec2(1, 56 * scale));
        }
        centred(label(e, "name") + " · " + label(e, "circuitName"), 21, palette().muted, 4);
        const int node = number(e, "lastNode", 8);
        if (node >= 8 && node <= 14) {
            const auto& wins = member(e, "wins")[node - 8];
            centred(std::string(node < 12   ? "四分之一决赛"
                                : node < 14 ? "半决赛"
                                            : "决赛") +
                        " · " + std::to_string(itemNumber(wins, 0, 0)) + "–" + std::to_string(itemNumber(wins, 1, 0)),
                    21, palette().muted, 4);
        }
        ImGui::Dummy(ImVec2(0, 16 * scale));
        if (champion) {
            centred(std::to_string(number(e, "purse")) + " 金币", 26, skin().value, 6);
            if (!upgrades) centred(label(e, "prize"), 21, palette().ink, 4);
            const auto p = ImGui::GetCursorScreenPos() + ImVec2(0, 15 * scale);
            int slot = 0;
            const auto& cards = arrayMember(e, "prizeCards");
            for (const auto& id : cards) {
                const auto pos = p + ImVec2(slot * 198 * scale, 0);
                ImGui::SetCursorScreenPos(pos);
                if (ImGui::InvisibleButton(("reward-art" + std::to_string(slot)).c_str(), ImVec2(180, 240) * scale,
                                           ImGuiButtonFlags_EnableNav))
                    inspectCard(id.get<int>());
                displayCard(id.get<int>(), pos, ImVec2(180, 240) * scale, false, -1, MaterialSlot::resultPrizes + slot);
                ++slot;
            }
            if (!cards.empty()) {
                ImGui::SetCursorScreenPos(p + ImVec2(0, 256) * scale);
                ImGui::Dummy(ImVec2(1, 1));
            }
        }
        if (upgrades) {
            textBlock(std::to_string(upgrades) + " 次闪卡升级机会剩余", width - 60 * scale, 21, palette().accent);
            std::vector<int> options;
            for (int i = 0; i < int(collection.cards.size()); ++i)
                if (collection.cards[i].owned[0] > 0 && collection.cards[i].owned[1] < 1000000) options.push_back(i);
            std::sort(options.begin(), options.end(),
                      [&](int a, int b) { return collection.cards[a].name < collection.cards[b].name; });
            if (std::find(options.begin(), options.end(), int(foilSelection)) == options.end())
                foilSelection = options.empty() ? 0 : options.front();
            std::vector<std::string> names;
            int selected = 0;
            for (int id : options) {
                if (id == int(foilSelection)) selected = int(names.size());
                names.push_back(collection.cards[id].name);
            }
            if (names.empty())
                textBlock("没有可升级的卡", 500 * scale, 18, palette().muted);
            else {
                // A grid to pick from, as in the album: a long dropdown can't be worked with a pad.
                const auto p = ImGui::GetCursorScreenPos();
                const float side = 230 * scale, gridWidth = width - 60 * scale - side - 20 * scale,
                            gridHeight = 380 * scale;
                ImGui::SetNextItemWidth(gridWidth);
                ImGui::InputTextWithHint("##foil-search", "Find a card", foilQuery, sizeof(foilQuery));
                std::vector<int> shown;
                for (int id : options)
                    if (!foilQuery[0] || lower(collection.cards[id].name).find(lower(foilQuery)) != std::string::npos)
                        shown.push_back(id);
                ImGui::SetCursorScreenPos(p + ImVec2(0, 46 * scale));
                ImGui::BeginChild("foil-cards", ImVec2(gridWidth, gridHeight - 46 * scale),
                                  ImGuiChildFlags_NavFlattened);
                const float gap = 8 * scale, tile = 86 * scale, tileHeight = tile / .75f;
                const int columns = std::max(1, int((gridWidth - 14 * scale + gap) / (tile + gap)));
                ImGuiListClipper clip;
                clip.Begin((int(shown.size()) + columns - 1) / columns, tileHeight + gap);
                while (clip.Step())
                    for (int row = clip.DisplayStart; row < clip.DisplayEnd; ++row) {
                        const auto at = ImGui::GetCursorScreenPos();
                        for (int col = 0; col < columns && row * columns + col < int(shown.size()); ++col) {
                            const int id = shown[row * columns + col];
                            const auto pos = at + ImVec2(col * (tile + gap), 0);
                            ImGui::PushID(id);
                            ImGui::SetCursorScreenPos(pos);
                            if (ImGui::InvisibleButton("foil-option", ImVec2(tile, tileHeight),
                                                       ImGuiButtonFlags_EnableNav))
                                foilSelection = id;
                            displayCard(id, pos, ImVec2(tile, tileHeight),
                                        int(foilSelection) == id ||
                                            (ImGui::IsItemFocused() && ImGui::GetIO().NavVisible),
                                        -1, MaterialSlot::none);
                            ImGui::PopID();
                        }
                        ImGui::SetCursorScreenPos(at + ImVec2(0, tileHeight + gap));
                        ImGui::Dummy(ImVec2(1, 0));
                    }
                if (shown.empty())
                    text(ImGui::GetCursorScreenPos() + ImVec2(0, 20 * scale), "没有这张卡", 18, palette().muted);
                ImGui::EndChild();
                // The chosen card, and the button that spends an upgrade on it.
                const auto chosen = p + ImVec2(gridWidth + 20 * scale, 0);
                displayCard(foilSelection, chosen, ImVec2(156, 208) * scale, false, -1, MaterialSlot::foilChoice);
                text(chosen + ImVec2(0, 216 * scale),
                     fitted(ttcg::zhCardName(collection.cards[foilSelection].name), 17, side), 18, palette().ink);
                if (button("upgrade", "制成闪卡", chosen + ImVec2(0, 250) * scale, ImVec2(200, 43) * scale, true,
                           !pendingCommand))
                    send("tournament", "foil " + std::to_string(eventID) + " " + std::to_string(foilSelection));
                ImGui::SetCursorScreenPos(p + ImVec2(0, gridHeight + 8 * scale));
                ImGui::Dummy(ImVec2(1, 1));
            }
        }
        const auto bottom = ImGui::GetWindowPos() + ImVec2(30 * scale, ImGui::GetWindowSize().y - 65 * scale);
        line(bottom - ImVec2(0, 15 * scale), bottom + ImVec2(width - 60 * scale, -15 * scale));
        const auto leave = [&](std::string action) {
            if (upgrades)
                confirm("还剩 " + std::to_string(upgrades) + " 次闪卡升级未使用，确定离开？", "这些机会将作废。",
                        "tournament", action + " " + std::to_string(eventID), "离开");
            else
                send("tournament", action + " " + std::to_string(eventID));
        };
        if (button("leave-tournament", "离开", bottom, ImVec2(110, 40) * scale)) {
            leave("leave");
            ImGui::CloseCurrentPopup();
        }
        if (!over && button("view-bracket", "查看对阵", bottom + ImVec2(125, 0) * scale, ImVec2(160, 40) * scale)) {
            leave("ack");
            ImGui::CloseCurrentPopup();
        }
        const auto next = over                                  ? champion ? "查看锦标赛" : "查看结果"
                          : announce == 1                       ? "进入半决赛"
                          : announce == 2                       ? "进入决赛"
                          : std::string_view(outcome) == "平局" ? "重新对局"
                                                                : "下一局";
        if (button("next-game", next, bottom + ImVec2(width - 330 * scale, 0), ImVec2(270, 40) * scale, true,
                   !pendingCommand)) {
            if (over)
                leave("ack");
            else
                send("tournament", "continue " + std::to_string(eventID));
            ImGui::CloseCurrentPopup();
        }
        if (backPressed()) {
            leave("ack");
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
}
void GameApp::takeTrade() {
    const auto& c = member(state, "collection");
    if (std::popcount(claimMask) != number(c, "required")) return;
    send(number(c, "trade") == 1 ? "claim" : "claim-many",
         std::to_string(number(c, "trade") == 1 ? std::countr_zero(claimMask) : claimMask));
}
void GameApp::resultModal() {
    if (label(state, "phase") != "result" || flag(state, "settling")) return;
    const auto& c = member(state, "collection");
    if (number(c, "tournamentID")) return;
    // Inspection pauses this presentation, never the settlement transaction.
    if (inspecting || showRuleReference || !confirmationTitle.empty()) {
        trade.started += ImGui::GetIO().DeltaTime;
        return;
    }
    const bool choice = flag(c, "reward"), paying = flag(c, "paying");
    const int outcome = number(c, "outcome", -1), required = number(c, "required");
    const auto& hands = member(state, "originalHands");
    const auto& owners = member(c, "transferOwners");
    std::array<int, 10> destinations{};
    int transfers = 0;
    for (int origin = 0; origin < 10; ++origin) {
        destinations[origin] = itemNumber(owners, origin, origin / 5);
        if (destinations[origin] != origin / 5) ++transfers;
    }
    const auto now = ImGui::GetTime();
    const auto advance = [&](TradeView::Phase phase) {
        trade.phase = phase;
        trade.started = now;
    };
    if (trade.phase == TradeView::None)
        advance(number(c, "trade") || number(c, "wager") ? TradeView::Gathering : TradeView::Done);
    if (trade.phase == TradeView::Gathering && now - trade.started >= 1.0)
        advance(choice ? TradeView::Selecting : TradeView::Reviewing);
    if (trade.phase == TradeView::Selecting && !choice) {
        trade.chose = true;
        advance(TradeView::Reviewing);
    }
    // The result panel shows what changed hands, so the review only waits where there is something
    // the player has not already seen: cards taken by a rule or by the opponent.
    const bool settled = !flag(c, "staked") && !paying;
    if (trade.phase == TradeView::Reviewing && settled && (trade.chose || transfers == 0 || trade.acknowledged))
        advance(TradeView::Done);
    const bool done = trade.phase == TradeView::Done;
    const bool can = flag(c, "canPlay");
    const std::string reason =
        done ? (can ? label(c, "notice")
                    : label(c, "playUnavailableReason", label(state, "opponent") + " 没有足够的卡牌对局。"))
             : std::string{};
    const auto* event = tournament(number(member(c, "tournaments"), "notification"));
    const float reasonHeight =
        reason.empty() ? 0 : font()->CalcTextSizeA(18 * scale, 1e5f, 490 * scale, reason.c_str()).y + 24 * scale;
    // The closing dialog says how the game ended and what changed hands.
    int won = 0, lost = 0;
    for (int origin = 0; origin < 10; ++origin) {
        if (origin >= 5 && destinations[origin] == 0) ++won;
        if (origin < 5 && destinations[origin] == 1) ++lost;
    }
    const int wager = number(c, "wager");
    std::string settlement;
    const auto cards = [](int n) { return std::to_string(n) + " 张卡"; };
    if (won) settlement = "你赢得 " + cards(won);
    if (lost) settlement = settlement.empty() ? "你失去 " + cards(lost) : settlement + "，失去 " + cards(lost);
    if (wager && outcome >= 0)
        settlement += (settlement.empty() ? std::string(outcome == 0 ? "你赢得 " : "你失去 ")
                                          : std::string(outcome == 0 ? "，并赢得 " : "，并失去 ")) +
                      std::to_string(wager) + " 金币";
    if (!settlement.empty()) settlement += "。";
    // The cards that changed hands are shown, not just counted.
    std::vector<int> wonCards, lostCards;
    for (int origin = 0; origin < 10; ++origin) {
        const int id = hands.is_array() ? itemNumber(hands[origin / 5], origin % 5) : -1;
        if (id < 0) continue;
        if (origin >= 5 && destinations[origin] == 0) wonCards.push_back(id);
        if (origin < 5 && destinations[origin] == 1) lostCards.push_back(id);
    }
    const float prizeW = 104 * scale, prizeH = prizeW / .75f;
    const int prizeRows = int(!wonCards.empty()) + int(!lostCards.empty());
    const float summaryHeight = (settlement.empty() ? 96 : 132) * scale + prizeRows * (prizeH + 46 * scale);
    const auto extent = done
                            ? ImVec2(660 * scale, 150 * scale + summaryHeight + reasonHeight + (event ? 78 * scale : 0))
                            : ImVec2(1220, 724) * scale;
    if (!ImGui::IsPopupOpen("Game result")) ImGui::OpenPopup("Game result");
    ImGui::SetNextWindowPos(frameAt + frameSize * .5f, ImGuiCond_Always, ImVec2(.5f, .5f));
    ImGui::SetNextWindowSize(extent);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32, 28) * scale);
    if (ImGui::BeginPopupModal("Game result", nullptr,
                               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                   popupFlags())) {
        const auto panel = ImGui::GetWindowPos();
        surface(panel, extent);
        auto* d = ImGui::GetWindowDrawList();
        const auto origin = panel + ImVec2(32, 28) * scale;
        const float width = extent.x - 64 * scale;
        if (done) {
            float y = origin.y + 4 * scale;
            const float middle = panel.x + extent.x / 2;
            const char* heading = outcome < 0 ? "平局" : outcome == 0 ? "胜利" : "失败";
            const ImU32 tone = outcome == 0 ? goldInk() : outcome < 0 ? palette().ink : skin().notice;
            {
                const float half = capsWidth(heading, 32, 5) * .5f + 26 * scale,
                            reach = std::max(0.f, extent.x * .5f - 44 * scale - half);
                textCaps(ImVec2(middle, y), heading, 32, tone, 5, true);
                gradientLine(d, ImVec2(middle - half - reach, y + 21 * scale), reach, fade(tone, .7f));
                gradientLine(d, ImVec2(middle + half, y + 21 * scale), reach, fade(tone, .7f));
            }
            const auto& score = member(state, "score");
            text(ImVec2(middle, y + 52 * scale),
                 std::to_string(itemNumber(score, 0)) + " – " + std::to_string(itemNumber(score, 1)), 26, palette().ink,
                 0, true);
            float rowY = y + 96 * scale;
            if (!settlement.empty()) {
                text(ImVec2(middle, rowY), settlement, 18, palette().muted, 0, true);
                rowY += 36 * scale;
            }
            int slot = 0;
            const auto prizes = [&](const std::vector<int>& cards, const char* caption, int owner) {
                if (cards.empty()) return;
                textCaps(ImVec2(middle, rowY), caption, 13, owner == 0 ? fade(tone, .9f) : palette().muted, 1.6f, true);
                const float gap = 12 * scale, total = cards.size() * prizeW + (cards.size() - 1) * gap;
                for (int i = 0; i < int(cards.size()); ++i) {
                    const ImVec2 pos(middle - total * .5f + i * (prizeW + gap), rowY + 26 * scale);
                    ImGui::PushID(slot);
                    ImGui::SetCursorScreenPos(pos);
                    if (ImGui::InvisibleButton("result-card", ImVec2(prizeW, prizeH), ImGuiButtonFlags_EnableNav))
                        inspectCard(cards[i], false);
                    softShadow(pos, ImVec2(prizeW, prizeH), 1, 7);
                    displayCard(cards[i], pos, ImVec2(prizeW, prizeH),
                                ImGui::IsItemFocused() && ImGui::GetIO().NavVisible, owner,
                                MaterialSlot::tradeHands + slot);
                    ImGui::PopID();
                    ++slot;
                }
                rowY += prizeH + 46 * scale;
            };
            prizes(wonCards, "获得", 0);
            prizes(lostCards, "失去", 1);
            y += summaryHeight;
            if (!reason.empty()) {
                text(ImVec2(panel.x + extent.x / 2, y), reason, 18, palette().muted, width, true);
                y += reasonHeight;
            }
            if (event) {
                text(ImVec2(panel.x + extent.x / 2, y), label(*event, "name"), 18, palette().accent, width, true);
                if (button("tournament-notice", "查看锦标赛",
                           ImVec2(panel.x + extent.x / 2 - 95 * scale, y + 28 * scale), ImVec2(190, 36) * scale)) {
                    eventID = number(*event, "id");
                    send("tournament", "board");
                    ImGui::CloseCurrentPopup();
                }
                y += 78 * scale;
            }
            const float total = (can ? 340 : 150) * scale;
            const auto buttons = ImVec2(panel.x + (extent.x - total) / 2, panel.y + extent.y - 80 * scale);
            if (can && button("rematch", "再来一局", buttons, ImVec2(172, 42) * scale, true, !pendingCommand)) {
                send("prepare");
                ImGui::CloseCurrentPopup();
            }
            if (button("leave", "离开", buttons + ImVec2(can ? 190 * scale : 0, 0), ImVec2(150, 42) * scale, !can) ||
                backPressed()) {
                send("close");
                ImGui::CloseCurrentPopup();
            }
        } else {
            std::string title = choice         ? required == 1 ? "选择一张卡"
                                                               : "选择 " + std::to_string(required) + " 张卡 · " +
                                                                     std::to_string(std::popcount(claimMask)) + "/" +
                                                                     std::to_string(required)
                                : outcome < 0  ? "平局"
                                : outcome == 0 ? "你赢了！"
                                               : label(state, "opponent") + "的选择";
            text(origin + ImVec2(width / 2, 0), title, 26, palette().ink, width, true);
            const float gap = 16 * scale, w = 160 * scale, h = w / .75f;
            std::array<ImVec2, 10> positions{};
            const float elapsed = float(now - trade.started), left = origin.x + (width - 5 * w - 4 * gap) * .5f;
            const bool selectable = trade.phase == TradeView::Selecting && !pendingCommand;
            for (int player = 0; player < 2; ++player) {
                const float y = origin.y + (64 + (1 - player) * 268) * scale;
                text(ImVec2(left, y), player ? label(state, "opponent") : "你", 21, palette().muted);
                for (int slot = 0; slot < 5; ++slot) {
                    const int index = player * 5 + slot, id = hands.is_array() ? itemNumber(hands[player], slot) : -1;
                    if (id < 0) continue;
                    const bool moving = destinations[index] != player;
                    const bool selected = choice ? player == 1 && (claimMask & (1u << slot)) : moving;
                    const auto pos = ImVec2(left + slot * (w + gap), y + 34 * scale);
                    positions[index] = pos;
                    ImGui::PushID(index);
                    ImGui::SetCursorScreenPos(pos - ImVec2(0, selected ? 14 * scale : 0));
                    const bool hit = ImGui::InvisibleButton("trade-card", ImVec2(w, h), ImGuiButtonFlags_EnableNav);
                    if (ImGui::IsItemHovered() || ImGui::IsItemFocused()) trade.inspected = index;
                    if (hit && selectable && player == 1) {
                        const auto bit = 1u << slot;
                        if (claimMask & bit)
                            claimMask &= ~bit;
                        else if (required == 1)
                            claimMask = bit;
                        else if (std::popcount(claimMask) < required)
                            claimMask |= bit;
                    }
                    if (inspectRequested() || (hit && (!choice || player == 0))) inspectCard(id);
                    const bool inFlight = trade.phase == TradeView::Gathering && (trade.sourceMask & (1u << index)) &&
                                          elapsed < .58 + index * .045;
                    if (!inFlight)
                        displayCard(id, pos - ImVec2(0, selected ? 14 * scale : 0), ImVec2(w, h), selected, player,
                                    MaterialSlot::tradeHands + index);
                    ImGui::PopID();
                }
            }
            auto footer = origin + ImVec2(0, extent.y - 96 * scale);
            line(footer, footer + ImVec2(width, 0));
            footer.y += 22 * scale;
            const int wager = number(c, "wager");
            if (wager)
                text(footer + ImVec2(0, 5 * scale),
                     (outcome < 0 ? std::to_string(wager) + " 金币已退回"
                                  : std::string(outcome == 0 ? "+" : "−") + std::to_string(wager) + " 金币"),
                     26, outcome == 1 ? lossInk() : palette().accent);
            if (!label(c, "notice").empty())
                text(footer + ImVec2(250 * scale, 8 * scale), label(c, "notice"), 15, palette().muted,
                     width - 610 * scale);
            else {
                // The dialog covers the window's own prompts, so it shows the ones that apply here.
                float x = footer.x + 250 * scale;
                x += keyPrompt("Mouse1", "360_A", "选择", ImVec2(x, footer.y + 5 * scale)) + 26 * scale;
                x += keyPrompt("Mouse2", "360_Y", "查看", ImVec2(x, footer.y + 5 * scale)) + 26 * scale;
                if (choice) text(ImVec2(x, footer.y + 11 * scale), "选择要带走的战利品。", 15, palette().muted);
            }
            if (button("trade-inspect", "查看", footer + ImVec2(width - 302 * scale, 0), ImVec2(126, 40) * scale)) {
                const int index = trade.inspected;
                inspectCard(itemNumber(hands[index / 5], index % 5));
            }
            if (choice || paying)
                if (button("take-cards", paying ? "重试" : "确认", footer + ImVec2(width - 150 * scale, 0),
                           ImVec2(150, 40) * scale, true,
                           paying || selectable && std::popcount(claimMask) == required)) {
                    if (paying)
                        send("payout");
                    else
                        takeTrade();
                }
            // Cards taken by a rule or by the opponent stay up until the player has seen them.
            if (trade.phase == TradeView::Reviewing && settled && !trade.chose && transfers > 0)
                if (button("trade-continue", "继续", footer + ImVec2(width - 150 * scale, 0), ImVec2(150, 40) * scale,
                           true))
                    trade.acknowledged = true;
            // Render the gathering flights above the panel, at full colour.
            d->PushClipRect(frameAt, frameAt + frameSize, false);
            for (int index = 0; index < 10; ++index) {
                const int player = index / 5, id = hands.is_array() ? itemNumber(hands[player], index % 5) : -1;
                if (id < 0) continue;
                if (trade.phase == TradeView::Gathering && (trade.sourceMask & (1u << index))) {
                    const float t = std::clamp((elapsed - index * .045f) / .58f, 0.f, 1.f),
                                ease = 1 - std::pow(1 - t, 3);
                    if (t < 1)
                        displayCard(id, trade.sources[index] + (positions[index] - trade.sources[index]) * ease,
                                    trade.sizes[index] + (ImVec2(w, h) - trade.sizes[index]) * ease, false, player,
                                    MaterialSlot::tradeHands + index);
                }
            }
            d->PopClipRect();
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
}
}
//（注：内容由AI生成）
