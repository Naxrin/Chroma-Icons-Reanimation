// this file the base and some defines for most common used cases
#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

// used for transition between pages
#define ANIM_TIME_L 0.1 * vals["anim-time"] + 0.2

// used in delay between old page fades out and new page fades in
#define ANIM_TIME_M 0.06 * vals["anim-time"] + 0.12

// used in scroller cell delay and item cell enter delay
#define ANIM_TIME_GAP 0.006 * vals["anim-time"] + 0.012

// used for cell colors
#define CELL_COLOR 255 * opts["dark-theme"], 255 * opts["dark-theme"], 255 * opts["dark-theme"]

// used for bg colors
#define BG_COLOR 255 * (1 - opts["dark-theme"]), 255 * (1 - opts["dark-theme"]), 255 * (1 - opts["dark-theme"])

/********** Useful Const Value *************/

// game manager
static GameManager* gm = GameManager::sharedState();

// fade utility (CCNode)
void fade(CCNode* node, bool in, float time, float scaleX = -1, float scaleY = -1, int opacity = -1);
// fade utility (CCMenuItem)
void fade(CCMenuItem* node, bool in, float time, float scaleX = -1, float scaleY = -1, int opacity = -1);

/********** Item Channel Enumerate Class *************/

// A single game mode (common, icon, ...)
// Icon is for easy mode common
enum class Gamemode {
    Icon, Cube, Ship, Ball, Ufo, Wave, Robot, Spider, Swing, Jetpack
};
// pick icon name in my own mod
static std::string items[] = {
    "Icon", "Cube", "Ship", "Ball", "Ufo", "Wave", "Robot", "Spider", "Swing", "Jetpack" // 0~9
};

// sprite channels
enum class Channel {
    Main, Secondary, Glow, White, Ghost, Trail, DashFire, TPLine, WaveTrail, UFOShell
};

// channel names regarding enum class above
static std::string chnls[] = {
    "Main", "Secondary", "Glow", "White",
    "Ghost Trail", "Trail", "Dash Fire", "TP Line", "Wave Trail", "UFO Shell"
};

/********** SETUP AND SERIELIZE ***********/

// for gradient mode, int means 0~359
// for prtogress mode, int means 0~100
typedef std::pair<int, ccColor3B> pairpos;
// madeline
typedef std::map<int, ccColor3B> mapline;

// fade a slider
void fadeSlider(Slider* slider, bool in);

// load int color array from json file
// @param def default value if fail to convert
// @param max 360 for gradient mode and 101 for progress mode
mapline MapfromJson(matjson::Value const& json, int def, int max);

// dump int color array to json file
matjson::Value MaptoJson(mapline const& vec);

// The whole set of a color channel config
struct ChromaPattern {
    // chroma mode Default/Static/Chromatic/Gradient/Progress
    int mode;
    // static color
    ccColor3B color;
    // chromatic phase offset
    int phase;
    // chroma saturation eg.100=rgb/50=pastel
    int satu;
    // chromatic brightness
    int brit;
    // reverse
    bool rev;    
    // gradient map
    mapline gradient;
    // progress map
    mapline progress;
    // current percentage or best progress for progress related mode
    bool best;    
};

// default chroma setup for this mod
#define DEFAULT_SETUP ChromaPattern{\
    .mode = 0,\
    .color = ccc3(255, 255, 255),\
    .phase = 0,\
    .satu = 50,\
    .brit = 100,\
    .rev = false,\
    .gradient = {{0, ccc3(255, 255, 255)}, {180, ccc3(255, 255, 255)}},\
    .progress = {{0, ccc3(255, 255, 255)}, {100, ccc3(255, 255, 255)}},\
    .best = false\
}
/*********** serialize ChromaPattern **********/
template<>
struct matjson::Serialize<ChromaPattern> {
    // load json file
    // for int value we take the remainder to avoid out of range issue
    static Result<ChromaPattern> fromJson(matjson::Value const& value) {
        return Ok(ChromaPattern{
            .mode = value.contains("mode") ? ((int)value["mode"].asInt().unwrapOr(0)) % 5 : 0,
            .color = value["color"].asString().andThen([](auto str) { return cc3bFromHexString(str, true); })
                .unwrapOr(ccc3(255,255,255)),
            .phase = value.contains("phase") ? ((int)value["phase"].asInt().unwrapOr(0)) % 360 : 0, 
            .satu = value.contains("saturation") ? ((int)value["saturation"].asInt().unwrapOr(50)) % 101 : 50,
            .brit = value.contains("brightness") ? ((int)value["brighttness"].asInt().unwrapOr(100)) % 101 : 100,
            .rev = value.contains("reverse") ? value["reverse"].asBool().unwrapOr(false) : false,
            .gradient = value.contains("gradient") ? MapfromJson(value["gradient"], 180, 360) : mapline({{0, ccc3(255, 255, 255)}, {180, ccc3(255, 255, 255)}}),
            .progress = value.contains("progress") ? MapfromJson(value["progress"], 100, 101) : mapline({{0, ccc3(255, 255, 255)}, {100, ccc3(255, 255, 255)}}),
            .best = value.contains("best") ? value["best"].asBool().unwrapOr(false) : false
        });
    }

    // dump to json file
    static matjson::Value toJson(ChromaPattern const& value) {
        return matjson::makeObject({
            {"mode", value.mode},
            {"color", cc3bToHexString(value.color)},
            {"phase", value.phase},
            {"saturation", value.satu},
            {"brightness", value.brit},
            {"reverse", value.rev},
            {"gradient", MaptoJson(value.gradient)},
            {"progress", MaptoJson(value.progress)},
            {"best", value.best}
        });
    }

    static bool isJson(matjson::Value value) {
        //return value.isObject();
        return true;
    }
};

/********** EVENT ***********/

// Integrated Event Signal Emitter for mod menu
template<typename T>
struct Signal : public Event<Signal<T>, bool(T), std::string> {
    using Event<Signal<T>, bool(T), std::string>::Event;
};

struct myColorHSV {
    float h;
    float s;
    float v;
};

// chroma engine
// @param setup the chroma pattern it refers to
// @param defaultVal for mode 0
// @param phase current phase
// @param percentage current level percentage
// @param progress current level progress
ccColor3B getChroma(ChromaPattern const& setup, ccColor3B const& defaultVal, float phase, float percentage, int8_t progress);

// get index for in-level pointer
// @param p2 is player 2
// @param id Gamemode ID regarding ChromaLayer's index 0~9
// @param channel main/second/glow/...
// @return short int result index
inline short getIndex(bool p2, Gamemode id, Channel channel) {
    return (short)p2 << 10 | (int)id << 5 | (int)channel;
}

// get config key for mod save container
// @param p2 is player 2
// @param id Gamemode ID regarding ChromaLayer's index 0~9
// @param channel main/second/glow/...
// @return string result
inline std::string getConfigKey(bool p2, Gamemode id, Channel channel) {
    return fmt::format("{}-{}-{}", p2 ? "P2" : "P1", items[(int)id], chnls[(int)channel]);
}