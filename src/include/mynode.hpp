// This file includes all basic inherited ui node classes
#pragma once

#include "utility.hpp"
#include <Geode/ui/TextInput.hpp>

inline float limiter(float target, float min = 0, float max = 1) {
    if (target > max) return max;
    if (target < min) return min;
    return target;
}

/********** UI ***********/

class PickItemButton : public CCMenuItemSpriteExtra {
protected:
    // this icon is from setup page or not
    bool src;
    // is player 2
    bool ptwo;
    // chroma
    bool chroma = false;
    // icon sprite (gamemode tab only)
    GJItemIcon* icon = nullptr;
    // effect sprite (effect tab only)
    GJItemEffect* effect = nullptr;
    // colors
    ccColor3B mainColor;
    ccColor3B secondColor;
    ccColor3B glowColor;
    // init
    bool init(int tag, bool src, CCObject* target, cocos2d::SEL_MenuHandler callback);
public:
    // set frame and default colors
    void switchPlayer() {
        this->ptwo = !ptwo;
        setPlayerStatus();
    }
    // set player status
    void setPlayerStatus();
    // fade one by one
    void delayFade(int delay, bool in);
    // chroma proxy
    void runChroma(float const& phase, float const& percentage, int const& progress);
    // for current item switch
    void toggleChroma();
    // toggle on or off chroma
    void toggleChroma(bool current);
    // edit mode target (effect only)
    void setModeTarget(Gamemode gamemode) {
        if (this->effect->effectType == Channel::WaveTrail && (int)gamemode) {
            this->effect->targetMode = Gamemode::Wave;
            return;
        }
        if (this->effect->effectType == Channel::UFOShell && (int)gamemode) {
            this->effect->targetMode = Gamemode::Ufo;
            return;
        }
        this->effect->targetMode = gamemode;
    }

    // constructor for effects
    static PickItemButton* create(int tag, bool src, CCObject* target, cocos2d::SEL_MenuHandler callback) {
        auto node = new PickItemButton();
        if (node && node->init(tag, src, target, callback)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

// a node with feedback logic design of a slider + input
class SliderBundleBase : public CCMenu, public TextInputDelegate, public SliderDelegate {
protected:
    // topic
    std::string topic;
    // value
    float value;
    // max value, -1 if up to infinity
    float max;
    // min value
    float min;
    // is int
    bool is_int;
    // label
    CCLabelBMFont* m_label = nullptr;
    // inputer
    TextInput* m_inputer = nullptr;
    // slider
    Slider* m_slider = nullptr;
    // left arrow
    CCMenuItemSpriteExtra* m_btnLeft = nullptr;
    // right arrow
    CCMenuItemSpriteExtra* m_btnRight = nullptr;    
    // value to slider (this may return a value out of 1~0 range, unfiltered)
    std::function<float (float)> toSlider;
    // slider to value
    std::function<float (float)> fromSlider;
    // init
    bool init(std::string topic, const char* title, float value, float max, float min, bool is_int, bool has_arrow, \
        float labelScale,  float sliderScale, float inputerScale, float arrowScale, float sliderPosX, float inputerPosX, float labelWidth, float inputerWidth, float arrowDistance,\
        std::function<float (float)> toSlider, std::function<float (float)> fromSlider);
public:
    // update value from text input
    void textChanged(CCTextInputNode* p) override {
        std::string input = p->getString();
        if (input != "")
            this->setVal(limiter(stof(input)), 1);
    }
    // check value > max case
    void textInputClosed(CCTextInputNode* p) override {
        std::string input = p->getString();
        if (input == "")
            input = "0";
        this->setVal(limiter(stof(input), min, max), 1);
        postEvent();
    }
    // change chroma frequency by slider
    void onSlider(CCObject* sender) {
        this->setVal(this->fromSlider(m_slider->getValue()), -1);
    }
    // on arrow
    void onArrow(CCObject* sender) {
        float delta = sender->getTag() == 2 ? 0.1 : -0.1;
        float news = limiter(this->toSlider(value) + delta);
        this->setVal(this->fromSlider(news));
        postEvent();
    }
    // mute onClose
    void sliderBegan(Slider *p) override {
        SignalEvent("drag-slider", true).post();
    };
    // unmute onClose and post event
    void sliderEnded(Slider *p) override {
        SignalEvent("drag-slider", false).post();
        this->setVal(this->fromSlider(m_slider->getValue()));
        postEvent();
    };
    // post event
    void postEvent() {
        if (this->is_int)
            SignalEvent(this->topic, (int)value).post();
        else
            SignalEvent(this->topic, value).post();
    }
    float getVal() {
        return this->value;
    }
    void setVal(float value, short mode = 0);
    // i hate cascading opacity
    void helpFade(bool in);
};

// speed slider in main menu
class SpeedSliderBundle : public SliderBundleBase {
protected:
    // init
    bool init();
public:
    // fade with item menu
    void Fade(bool in);

    static SpeedSliderBundle* create(float val) {
        auto node = new SpeedSliderBundle();
        if (node && node->init()) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

enum class OptionLineType {
    Title = 1,
    Desc = 2,
    SingleColor = 3,
    MultiColor = 4,
    Slider = 5,
    Toggler = 6
};

// Setup Option Line, not a cell
class SetupOptionLine : public SliderBundleBase {
protected:
    // init
    bool init(OptionLineType type, int mode, int tag);
    // toggler callback
    void onToggle(CCObject*) {
        // mode
        if (type == OptionLineType::Title)
            SignalEvent("mode", mode).post();
        // best toggler
        else if (type == OptionLineType::Toggler)
            SignalEvent("best", !m_toggler->isToggled()).post();
    };
    void onPickColor(CCObject* sender) {
        SignalEvent("color", sender->getTag()).post();
    };
public:
    // line ui type
    OptionLineType type;
    // the mode this line points to
    int mode;
    // title (title line only)
    CCLabelBMFont* m_title = nullptr;
    // toggler (shared)
    CCMenuItemToggler* m_toggler = nullptr;
    // color picker
    CCMenuItemSpriteExtra* m_colpk = nullptr;
    // color picker1
    CCMenuItemSpriteExtra* m_colpk1 = nullptr;
    // color picker2
    CCMenuItemSpriteExtra* m_colpk2 = nullptr;
    // toggle the toggler and set/tint the label' color
    void toggleTitle(bool yes, bool fade = false) {
        if (m_toggler)
            m_toggler->toggle(yes && !fade);
        if (type == OptionLineType::Title && m_title)
            // green or gray
            m_title->runAction(CCTintTo::create(fade*ANIM_TIME_M, 127-127*yes, 127+128*yes, 127-127*yes));
    }

    static SetupOptionLine* create(OptionLineType type, int mode, int tag) {
        auto node = new SetupOptionLine();
        if (node && node->init(type, mode, tag)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

class MyContentLayer : public GenericContentLayer {
public:
    // height of his scrollLayer mommy
    float ceilingHeight = 0;
    // nodes' Y positions when they are on display
    // arranged by tag order, from bottom to top
    std::vector<float> Ystd;
    // node action volume
    std::vector<CCAction*> acts;
    // get a mysterious value related to manually scrolled animaiton
    // @param Y cell's vertical pos
    // @param H cell's content height
    // @return opacity = 255*tg   scale = 0.5+0.5*tg
    float getSomething(float Y, float H);
    // override set position to meanwhile set their scale and opacity
    void setPosition(CCPoint const& pos) override {
        CCLayerColor::setPosition(pos);

        for (auto child : CCArrayExt<CCMenu*>(m_pChildren)) {
            float tg = getSomething(child->getPositionY(), child->getContentHeight());
            child->setVisible(tg > 0);
            child->setOpacity(255 * tg);
            child->setScale(0.5 + tg/2);
        }
    }
    // register the node's initial position Y
    void addChild(CCNode* child) override {
        CCNode::addChild(child);
        Ystd.push_back(child->getPositionY());
        acts.push_back(nullptr);
    }

    static MyContentLayer* create(float width, float height) {
        auto ret = new MyContentLayer();
        if (ret->initWithColor({ 0, 0, 0, 0 }, width, height)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class ScrollLayerPlus : public ScrollLayer {
protected:
    
    CCAction* actionFade = nullptr;
public:

    ScrollLayerPlus(CCRect const& rect, bool scrollWheelEnabled, bool vertical) : 
        ScrollLayer(rect, scrollWheelEnabled, vertical) {

        m_contentLayer->removeFromParent();
        m_contentLayer = MyContentLayer::create(rect.size.width, rect.size.height);
        m_contentLayer->setID("content-layer");
        m_contentLayer->setAnchorPoint({ 0, 0 });
        this->addChild(m_contentLayer);

        this->setMouseEnabled(true);
        this->setTouchEnabled(true);
    }
    void setCeiling() {
        static_cast<MyContentLayer*>(this->m_contentLayer)->ceilingHeight = getContentHeight();
    }
    void Transition(bool in, int move);

    static ScrollLayerPlus* create(CCRect const& rect, bool scroll = true, bool vertical = true) {
        auto ret = new ScrollLayerPlus(rect, scroll, vertical);
        ret->autorelease();
        return ret;
    }
};

