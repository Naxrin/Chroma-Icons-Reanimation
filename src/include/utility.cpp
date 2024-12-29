#include "utility.hpp"

// is player 2
bool ptwo = false;
// level
GJGameLevel* Level;
// get level prrogress
float progress;
// phase for game layer update
// mod menu has its own phase standalone
float lvlphase = 0.f;
// reset for each object
std::map<PlayerObject*, bool> reset;
// bools
std::map<std::string, bool> opts;
// speed option
float speed;
// setup center
std::map<short, ChromaSetup> setups;

void fade(CCNode* node, bool in, float time, float scaleX, float scaleY, int opacity) {
    // convert default Val
    if (opacity < 0)
        opacity = in ? 255 : 0;
    if (scaleX < 0)
        scaleX = in ? 1 : 0.5;
    if (scaleY < 0)
        scaleY = in ? 1 : 0.5;

    auto action = CCSpawn::create(
        CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, opacity)),
        CCEaseExponentialOut::create(CCScaleTo::create(ANIM_TIME_M, scaleX, scaleY)),
        nullptr
    );
    if (in)
        node->setVisible(true);

    node->runAction(CCSequence::create(action,
        CallFuncExt::create([node, in](void) { node->setVisible(in); }),
        nullptr));
}

void fade(CCMenuItem* node, bool in, float time, float scaleX, float scaleY, int opacity) {
    // convert default Val
    if (opacity < 0)
        opacity = in ? 255 : 0;
    if (scaleX < 0)
        scaleX = in;
    if (scaleY < 0)
        scaleY = in;

    auto action = CCSpawn::create(
        CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, opacity)),
        CCEaseExponentialOut::create(CCScaleTo::create(ANIM_TIME_M, scaleX, scaleY)),
        nullptr
    );
    if (in)
        node->setVisible(true);
    else
        node->setEnabled(false);

    node->runAction(CCSequence::create(action,
        CallFuncExt::create([node](void) { node->setEnabled(true); }),
        nullptr));
}


// load int color array from json file
mapline MapfromJson(matjson::Value const& json, int def, int max) {
    if (json.isNull())
        return {};
    if (!json.isObject())
        return {{0, ccc3(255, 255, 255)}, {def, ccc3(255, 255, 255)}};
    mapline map;
    for (auto& [key, val] : json)
        // regex check to avoid stoi crash, meanwhile block negative value
        // also kick out out of range value
        if (std::regex_match(key, std::regex("[0-9]+")) && stoi(key) < max && val.isString())
            map[stoi(key)] = val.asString().andThen([](auto str)
                { return cc3bFromHexString(str, true); }).unwrapOr(ccc3(255,255,255));
    return map;
}

// dump int color array to json file
matjson::Value MaptoJson(mapline const& map) {
    matjson::Value json;
    for (auto pair: map)
        json[std::to_string(pair.first)] = cc3bToHexString(pair.second);
    return json;
}

GJItemEffect* GJItemEffect::createEffectItem(int effectID) {
    auto base = new GJItemEffect();
    if (base && base->initWithFile("effect_base.png"_spr)) {
        // cascade opacity
        base->setCascadeOpacityEnabled(true);

        // add cover
        std::string str = "effect_"_spr + std::to_string(effectID) + ".png";
        base->m_cover = CCSprite::create(str.c_str());
        auto size = base->getContentSize();
        base->m_cover->setPosition(CCPoint(size.width / 2, size.height / 2));
        base->m_cover->setID("cover");
        base->addChild(base->m_cover);

        base->autorelease();
        return base;
    }
    CC_SAFE_DELETE(base);
    return nullptr;
}

// RGB -> HSV
// copy from gay wave trail
inline ccColor3B HSVtoRGB(float h, float s, float v) {
    float c = v * s;
    float hp = fmod(h / 60.0, 6);
    float x = c * (1 - fabs(fmod(hp, 2) - 1));
    float m = v - c;

    float r, g, b;
    if (0 <= hp && hp < 1) {
        r = c; g = x; b = 0;
    } else if (1 <= hp && hp < 2) {
        r = x; g = c; b = 0;
    } else if (2 <= hp && hp < 3) {
        r = 0;  g = c; b = x;
    } else if (3 <= hp && hp < 4) {
        r = 0;  g = x; b = c;
    } else if (4 <= hp && hp < 5) {
        r = x; g = 0;  b = c;
    } else if (5 <= hp && hp < 6) {
        r = c; g = 0;  b = x;
    } else {
        r = 0;  g = 0;  b = 0;
    }
    r += m;
    g += m;
    b += m;

    return ccc3(r*255, g*255, b*255);
}

// RGB -> HSV
// reverse of the function above
/*
inline ccHSVValue RGBtoHSV(ccColor3B rgb) {
    float delta, h, s, v;

    float r = rgb.r, g = rgb.g, b = rgb.b;
    if (r >= g && r >= b) {
        v = r;
        delta = g >= b ? r-b : r-g;
        h = 60 * fmod((g-b)/(delta), 6);
    }
    else if (g >= b && g >= r) {
        v = g;
        delta = b >= r ? g-r : g-b;
        h = 60 * ((b-r)/delta + 2);
    }
    else if (b >= r && b >= g){
        v = b;
        delta = r >= g ? b-g : b-r;
        h = 60 * ((r-g)/delta + 4);
    }
    s = v == 0 ? 0 : delta / v;
    return ccHSVValue{h, s, v, true, true};
}*/

// get RGB cycle color
inline ccColor3B getRainbow(float hue, float saturation) {
    return HSVtoRGB(hue, saturation / 100, 1);
}

inline ccColor3B getGradient(const float &middle, const pairpos &l, const pairpos &r) {
    float p = (middle - l.first) / (r.first - l.first);
    return ccc3(
        l.second.r + p * (r.second.r - l.second.r),
        l.second.g + p * (r.second.g - l.second.g),
        l.second.b + p * (r.second.b - l.second.b)
    );
}

ccColor3B getChroma(ChromaSetup const& setup, ccColor3B const& defaultVal, float phase, float progress, bool reset) {
    if (reset)
        return defaultVal;

    // current left intcolor valve
    pairpos l;
    bool start = true;
    switch (setup.mode) {
    // static
    case 1:
        return setup.color;
    // chromatic
    case 2:
        return getRainbow(phase, setup.satu);
    // gradient
    case 3:
        // dont crash the game at least
        if (setup.gradient.empty()) {
            return defaultVal;
        }
        for (pairpos r : setup.gradient) {
            if (r.first > phase) {
                pairpos ret = std::make_pair(setup.gradient.end()->first - 360, setup.gradient.end()->second);
                return getGradient(phase, l, start ? ret : r);
            }
            // this may be the left
            l = r;
            start = false;
        }

        return getGradient(phase, l, std::make_pair(setup.gradient.begin()->first + 360, setup.gradient.begin()->second));
    case 4:
        // dont crash the game at least
        if (setup.progress.empty()) {
            return defaultVal;
        }
        for (pairpos r : setup.progress) {
            if (r.first > progress)
                return start ? setup.progress.begin()->second : getGradient(progress, l, r);
            // this may be the left
            l = r;
            start = false;
        }
        return setup.progress.end()->second;
    // default
    default:
        return defaultVal;
    }
}