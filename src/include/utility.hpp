// this file is used for most common used cases
// also include some defines
#pragma once

#include <Geode/Geode.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <regex>

using namespace geode::prelude;

// used for transition between pages
// @note 0 - 0.5
#define ANIM_TIME_L Mod::get()->getSavedValue<float>("anim-speed", 0.4)
// used in delay between old page fades out and new page fades in
// @note 0 - 0.24
#define ANIM_TIME_M 0.6 * Mod::get()->getSavedValue<float>("anim-speed", 0.4)
// used in scroller cell delay adn item cell enter delay
// @note 0 - 0.05
#define ANIM_TIME_GAP 0//.1 * Mod::get()->getSavedValue<float>("anim-speed", 0.4)

#define CELL_COLOR Mod::get()->getSavedValue<bool>("dark-theme", true) ? 255 : 0, \
                    Mod::get()->getSavedValue<bool>("dark-theme", true) ? 255 : 0, \
                    Mod::get()->getSavedValue<bool>("dark-theme", true) ? 255 : 0

#define BG_COLOR Mod::get()->getSavedValue<bool>("dark-theme", true) ? 0 : 255, \
                    Mod::get()->getSavedValue<bool>("dark-theme", true) ? 0 : 255, \
                    Mod::get()->getSavedValue<bool>("dark-theme", true) ? 0 : 255

/********** Useful Const Value *************/

// pick icon name in my own mod
static std::string items[] = {
    "Icon", "Cube", "Ship", "Ball", "Ufo", "Wave", "Robot", "Spider", "Swing", "Jetpack", // 0~9
    "Ghost", "Trail", "Wave Trail", "Dash Fire", "TP Line", "UFO Shell" // 10~15
};

// channel names
static std::string chnls[] = {
    "Main", "Secondary", "Glow", "White", "Effect"
};

// game manager
static GameManager* gm = GameManager::sharedState();

// fade utility (CCNode)
void fade(CCNode* node, bool in, float time = ANIM_TIME_L, float scaleX = -1, float scaleY = -1, int opacity = -1);
// fade utility (CCMenuItem)
void fade(CCMenuItem* node, bool in, float time = ANIM_TIME_L, float scaleX = -1, float scaleY = -1, int opacity = -1);

// channels of a single game mode (common, icon, ...)
// ghost~Tele are effect channels, ghost is preserved for future update
// effect is just wave trail and ufo shell
enum class Channel {
    Main, Secondary, Glow, White, Effect, Cube, Ship, Ball, Ufo, Wave, Robot, Spider, Swing, Jetpack
};


/********** SETUP AND SERIELIZE ***********/
typedef std::map<int, ccColor3B> mapline;
typedef std::pair<int, ccColor3B> pairpos;

// load int color array from json file
mapline MapfromJson(matjson::Value const& json, int def, int max);

// dump int color array to json file
matjson::Value MaptoJson(mapline const& vec);

// The whole set of a color channel config
struct ChromaSetup {
    // chroma mode Default/Static/Chromatic/Gradient/Progress
    int mode;
    // chroma saturation eg.100=rgb/50=pastel
    int satu;
    // current/best progress for progress related mode
    bool best;
    // static color
    ccColor3B color;
    // gradient map
    mapline gradient;
    // progress map
    mapline progress;
};

// default chroma setup for this mod
#define DEFAULT_SETUP ChromaSetup{\
    .mode = 0, .satu = 50, .best = false, .color = ccc3(255, 255, 255),\
    .gradient = {{0, ccc3(255, 255, 255)}, {180, ccc3(255, 255, 255)}},\
    .progress = {{0, ccc3(255, 255, 255)}, {100, ccc3(255, 255, 255)}} }

/*********** serialize chromasetup **********/
template<>
struct matjson::Serialize<ChromaSetup> {

    // load json file
    // for int value we take the remainder to avoid out of range issue
    static Result<ChromaSetup> fromJson(matjson::Value const& value) {
        return Ok(ChromaSetup{
            .mode = ((int)value["mode"].asInt().unwrapOr(0)) % 5,
            .satu = ((int)value["saturation"].asInt().unwrapOr(50)) % 101,
            .best = value["best"].asBool().unwrapOr(false),
            .color = value["color"].asString().andThen([](auto str) { return cc3bFromHexString(str, true); })
                .unwrapOr(ccc3(255,255,255)),
            .gradient = MapfromJson(value["gradient"], 180, 360),
            .progress = MapfromJson(value["progress"], 100, 101)
        });
    }

    // dump to json file
    static matjson::Value toJson(ChromaSetup const& value) {
        return matjson::makeObject({
            {"mode", value.mode},
            {"saturation", value.satu},
            {"best", value.best},
            {"color", cc3bToHexString(value.color)},
            {"gradient", MaptoJson(value.gradient)},
            {"progress", MaptoJson(value.progress)}
        });
    }

    // judge json file with max tolerance
    static bool isJson(matjson::Value value) {
        if (!value.isObject()) return false;
        if (!value.contains("mode") || !value["mode"].isNumber()) return false;
        if (!value.contains("saturation") || !value["saturation"].isNumber()) return false;
        if (!value.contains("best") || !value["best"].isBool()) return false;
        if (!value.contains("color") || !value["color"].isString()) return false;
        if (!value.contains("gradient") || !value["gradient"].isObject()) return false;
        if (!value.contains("progress") || !value["progress"].isObject()) return false;
        return true;
    }
};


/********** EVENT ***********/
// Integrated Event Signal Emitter
template<typename T>
class SignalEvent : public Event {
public:
    std::string name;
    T value;
    SignalEvent(std::string name, T value) : Event() {
        this->name = name;
        this->value = value;
    }
};

enum class EffectType {
    Ghost, Trail, WaveTrail, DashFire, Teleport, Shell
};

class GJItemEffect : public CCSprite {
public:
    EffectType type;
    // 4~12
    int targetMode = 4;
    CCSprite* m_cover;
    static GJItemEffect* createEffectItem(int effectID);
};

struct myColorHSV {
    float h;
    float s;
    float v;
};

// chroma engine
ccColor3B getChroma(ChromaSetup const& setup, ccColor3B const& defaultVal, float phase, float percentage, float progress, bool reset = false);

// get index for in-level pointer
// @param p2 is player 2
// @param id Icon/Effect ID regarding ChromaLayer's index (0~9, 10~15)
// @param channel for icons they are main/second/glow, for effects they are icon targets index or just effect(10)
// @return short int result 10000100
inline short getIndex(bool p2, int id, int channel = 4) {
    return (short)p2 << 10 | id << 5 | channel;
}

// get config key for mod save
// @param p2 is player 2
// @param id Icon/Effect ID regarding ChromaLayer's index (0~9, 11~15)
// @param channel for icons they are main/second/glow, for effects they are icon targets index or just effect(10)
// @return string result
inline std::string getConfigKey(bool p2, int id, int channel = 4) {
    // get string
    std::string p = p2 ? "P2" : "P1";
    // concatenate
    if (id > 9) {
        // wave trail / ufo shell
        if (id == 12 || id == 15)
            return p + "-" + items[id];
        // should separate for icons
        else
            return p + "-" + items[id] + "-" + items[channel - 4];
    }
    // icons
    else
        return p + "-" + items[id] + "-" + chnls[channel];
}