#pragma once

#include <imgui.h>

namespace preview {

// Glyph coverage for the Chinese interface: Latin and punctuation from the common
// Chinese set, plus every extra character the translated strings use. The extra text
// is filled in from the translations; ImGui's common set alone misses rarer characters
// (the card names in particular) and would draw them as blanks.
inline const char* uiExtraText() {
    // Every non-ASCII character used by the Chinese translation, so nothing in the
    // interface, rule book or card list is drawn as an empty box.
    return
        "·×φ–—’“”…←↑→↓−♪、。一丁万三上下不与专世丘东丝两个中丹为主乃么义之乌乐也习书买乱了事二于云互五亚些亡交产亨亮亲人什仅仆仍从他付代令以仪们件任份伊伍伏伐休会传伤伦伯但位低住佐佑体何余作你佣佩使侍侠侧侵保信修俱候借值偏停傲像僵儿允元兄先光克入全八公兰共关兵其典兹兽内册再写军冠冥冰决况净准减凑凯凶出击函刀刃分切"
        "列则刚初删判利到制刷刺刽剂削前剑剧剩剪副割剿力功加动努劫励勒包匕化北匠匹区医十升半卓单博占卡卢卫印即卷历压原厨厩去参又及友双反发取受变叛叠口古另只召可台史右叶号司各合吉同名后向吗君含听启吸员命和咏咒哀品哈响哥哨哪唤售唯商啸喉嗜器噬囚四回因团园国图圣在地场坎坑坚坦型埃城域基堕堡塔塞境墓墙士壮声备复夏外多夜够大天夫失头夸夹"
        "夺奇奈奎奖套奥女好如妇妖妹姆始姐威娜婪嫌子字存学宁它守安完宗官定实审客室宫家容宽密寡对寻导射将尊小少尔尖尘尚尝尤就尸尼尾局层居屏展属屠山岛岩岸峭峰崖巅川巡巢工左巧巨巫差己已巴币布帅帆师希帕帝带帮常幕平年并幻幼幽序库应店废度庭康廷建开异弃式弓引弗弟张弩弱强归当录形彩影役征径徒得御微德徽心必志忘忠怎怒思性怯总恐恢恩息恶悍悲"
        "情惑惧想意懦戈戏成我戒或战截所扉手才扎扑扒打托执扩找技把投折护报披抽拉拒拓拔拖招拥择括拾拿持指按挑挡挥捍捕损换捣捷排掘掠探接控推描提揭搜携摆摇摩撑撤播操擎支收改攻放政故效敌敏救教散敦数整文斗斜斥斧断斯新方旁旅旋族旗无日旧早时昂昆明昏易星映是显晨普景晶智暂暗暮暴曲更曼替最月有服望朝期木未本术朱机朽杀权杆村杜束条来杯杰松板"
        "构林果枯柄某染柔查标树栖栗校株样核根格桃框桌桑档梅梦梧梭械检棋棕棘森棱槽模次歇歌止正此步武死殇残段殿毁母每毒比毛毡氏民水永汁求汉汗沃沉沙没河治沼沿法波泥注泰泳泽洛洞流济海涅消液深混添清渐渗游湮湾溪满漫潜潮瀑火灭灯灰灵灾炉炭炮点炼烬焦焰然照熊熔熟爆爪爬爱爵片版牌牙牛牧物特犬犯狂狗狡独狮狼猎率王玛玩玫现珀班球理琥琳瑞瑟瑰瓜"
        "瓦生用由甲电男画界畏畔留疗疫疯疾痛癫登白百的皇皮益盔盖盗盘盟目盲直相盾看眼着睡督睿矛知矮石砧础确碾磨示祖神祭祸禁离私秃种科秘秩称移稀程穆穴空穿突窃窗立竞章童端符第等答筛签算管篮籍米类粗精糊糖系素索紧紫红约级纯纳纵纸纹纺纽线练组绅细织终经绑结绘给绝统继绩绪续维绿缄编缘缚缺罐罗置羊羞群翅翻翼老考者而耐耶聆联肖肤胆背胜胡能脉"
        "脊腐自至舌舒舞航船色艾芙芬花苇苍苏若苦英荆草荒药荷莉莫莱莲获菲萎营萨落蒂蒙蒸蓝蕨蕾薄藏藤虎虫蚀蛇蛋蛛蛮蛾蜘蜜蜥蝎蝠蟹蠕血行衡补衬衮被裂装裔褐西要覆见观规觅视觉角解触言誓警计认让训议记许识诈试诗诚话该语诱说请诺课谁调谈谋谢谷象豪豸贝负贤货贩贪购贴贵费贼赃赌赛赞赢赫赭走起超足跚距跟路踪蹒躁身躯车转轮软轴较辉辑输辛辜辰边达过"
        "返还这进远连迪迹追退逆选透途通造道遗邀那邻郎部都酋配酒酬醒里重野量金针钢铁铜铭铳铸铺销锁锋锤锦键锻镇长门闪闭闲间队阴阵阶阻阿附际降限院除险隆随隐难雄集雕雨雪雷雾需霜霸非靠面革鞘音页项顺须顿预领题颚风食饕饥饱馆馏首马驱验骑骗骤骨骷髅高鬃鬼魁魂魅魇魔鱼鲁鳍鳞鸟鸣鹫鹰麦黄黎黑默黠鼎鼠齐齿龙（），：；？"
        ;
}

inline const ImWchar* uiGlyphRanges() {
    static ImVector<ImWchar> ranges;
    if (ranges.empty()) {
        ImFontGlyphRangesBuilder builder;
        builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
        builder.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesDefault());
        builder.AddText(uiExtraText());
        builder.BuildRanges(&ranges);
    }
    return ranges.Data;
}

}  // namespace preview
