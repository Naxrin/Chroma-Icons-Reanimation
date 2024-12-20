// this file is used for most common used cases
// also include some defines
#pragma once

#include <Geode/Geode.hpp>
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
#define ANIM_TIME_GAP 0.1 * Mod::get()->getSavedValue<float>("anim-speed", 0.4)

#define CELL_COLOR Mod::get()->getSavedValue<bool>("dark-theme", true) ? 255 : 0, \
                    Mod::get()->getSavedValue<bool>("dark-theme", true) ? 255 : 0, \
                    Mod::get()->getSavedValue<bool>("dark-theme", true) ? 255 : 0

#define BG_COLOR Mod::get()->getSavedValue<bool>("dark-theme", true) ? 0 : 255, \
                    Mod::get()->getSavedValue<bool>("dark-theme", true) ? 0 : 255, \
                    Mod::get()->getSavedValue<bool>("dark-theme", true) ? 0 : 255

/********** Useful Const Value *************/
// UnlockType tags by RobTop
static int gametype[9] = {1, 4, 5, 6, 7, 8, 9, 13, 14};

// pick icon/effect name in my own mod
// @warning icons and effects are different in indexing
static std::string items[15] = {
    "Icon", "Cube", "Ship", "Ball", "Ufo", "Wave", "Robot", "Spider", "Swing", "Jetpack",
    "Trail", "Wave Trail", "Dash Fire", "TP Line", "UFO Shell"
};

// channel names
static std::string chnls[3] = {"Main", "Secondary", "Glow"};

//player mode
static GameManager* gm = GameManager::sharedState();
static int garageitem[9] = {
    gm->getPlayerFrame(), gm->getPlayerShip(), gm->getPlayerBall(),
    gm->getPlayerBird(), gm->getPlayerDart(), gm->getPlayerRobot(),
    gm->getPlayerSpider(), gm->getPlayerSwing(), gm->getPlayerJetpack(),
};

// HSV -> RGB
// copy from gay wave trail
void HSVtoRGB(float &fR, float &fG, float &fB, float &fH, float &fS, float &fV);

// RGB -> HSV
// reverse of the function above
void RGBtoHSV(float &fR, float &fG, float &fB, float &fH, float &fS, float &fV);

// get RGB cycle color
ccColor3B getRainbow(float &phase, float offset, float saturation);

// fade utility (CCNode)
void fade(CCNode* node, bool in, float time = ANIM_TIME_L, float scaleX = -1, float scaleY = -1, int opacity = -1);
// fade utility (CCMenuItem)
void fade(CCMenuItem* node, bool in, float time = ANIM_TIME_L, float scaleX = -1, float scaleY = -1, int opacity = -1);

enum class Channel {
    Main = 0,
    Secondary = 1,
    Glow = 2,
    Effect = 3
};

/********** SETUP AND SERIELIZE ***********/
// percentage int and ccColor3B color
struct IntColor {
    int p;
    ccColor3B c;
};

// load int color array from json file
static std::vector<IntColor> IntColorfromJson(matjson::Value const& json, int defP) {
    if (!json.isObject())
        return {IntColor{.p = 0, .c = ccc3(255, 255, 255)}, IntColor{.p = defP, .c = ccc3(255, 255, 255)}};
    std::vector<IntColor> ret;
    for (auto& [key, val] : json) {
        // regex check to avoid stoi crash
        if (!std::regex_match(key, std::regex("[0-9]+")))
            continue;
            
        // goat element
        if (ret.empty()) {
            ret.push_back(IntColor{
                .p = stoi(key), 
                .c = val.asString().andThen([](auto str) { return cc3bFromHexString(str, true); }).unwrapOr(ccc3(255,255,255))});
            continue;
        }
        // find his place
        for (int i = 0; i < size(ret); i++)
            if (stoi(key) < ret.at(i).p) {
                ret.insert(ret.begin() + i, IntColor{
                .p = stoi(key), 
                .c = val.asString().andThen([](auto str) { return cc3bFromHexString(str, true); }).unwrapOr(ccc3(255,255,255))}
                );
                break;
            }
        // giant key element
        ret.push_back(IntColor{
            .p = stoi(key), 
            .c = val.asString().andThen([](auto str) { return cc3bFromHexString(str, true); }).unwrapOr(ccc3(255,255,255))});
    }
    return ret;
}

// dump int color array to json file
static matjson::Value IntColortoJson(std::vector<IntColor> const& vec) {
    auto json = matjson::makeObject({});
    for (auto obj: vec)
        json[std::to_string(obj.p)] = cc3bToHexString(obj.c);
    return json;
}

// judge json file with max tolerance
static bool isJson(matjson::Value value) {
    return true;
    if (!value.isObject()) return false;
    if (!value.contains("pos") || !value["pos"].isNumber()) return false;
    if (!value.contains("color") || !value["color"].isString()) return false;
    return true;
}

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
    // gradient vector
    std::vector<IntColor> gradient;
    // progress vector
    std::vector<IntColor> progress;
};

// default chroma setup for this mod
#define DEFAULT_SETUP ChromaSetup{\
    .mode = 0, .satu = 50, .best = false, .color = ccc3(255, 255, 255),\
    .gradient = {IntColor{.p = 0, .c = ccc3(255, 255, 255)}, IntColor{.p = 50, .c = ccc3(255, 255, 255)}},\
    .progress = {IntColor{.p = 0, .c = ccc3(255, 255, 255)}, IntColor{.p = 100, .c = ccc3(255, 255, 255)}} }

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
            .gradient = IntColorfromJson(value["gradient"], 50),
            .progress = IntColorfromJson(value["progress"], 100)
        });
    }

    // dump to json file
    static matjson::Value toJson(ChromaSetup const& value) {
        return matjson::makeObject({
            {"mode", value.mode},
            {"saturation", value.satu},
            {"best", value.best},
            {"color", cc3bToHexString(value.color)},
            {"gradient", IntColortoJson(value.gradient)},
            {"progress", IntColortoJson(value.progress)}
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