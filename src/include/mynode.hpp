// This file includes all basic inherited ui node classes
#pragma once

#include "utility.hpp"
#include <Geode/ui/TextInput.hpp>
#include <Geode/loader/Dispatch.hpp>

/********** UI ***********/

// rewrite GJItemIcon to cascade its opacity
class GJItemIconAlpha : public GJItemIcon {
public:
    static GJItemIconAlpha* createBrowserItem(int typeID, int iconID);
};

class GJItemEffect : public CCSprite {
public:
    CCSprite* m_cover;
    static GJItemEffect* createEffectItem(int effectID);
};

class PickItemButton : public CCMenuItemSpriteExtra {
protected:
    bool p2 = false;
    CCSprite* spr = nullptr;
public:
    void delayFade(int delay, bool in);
    void chroma(ccColor3B main, ccColor3B secondary, ccColor3B glow) {}
    void chroma(ccColor3B color) {}
    void switchPlayer() {
        this->p2 = !p2;
    }
    // constructor for effects
    static PickItemButton* create(CCSprite* spr, CCObject* target, cocos2d::SEL_MenuHandler callback) {
        auto node = new PickItemButton();
        node->spr = spr;
        if (node && node->init(node->spr, node->spr, target, callback)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

// a feedback logic design of a slider + input
class SliderBundleBase : public CCMenu, public TextInputDelegate, public SliderDelegate {
protected:
    // value
    float value;
    // max value, -1 if up to infinity
    float max;
    // init
    bool init(const char* name, float value, float max);
public:
    // nodes
    CCLabelBMFont* m_label = nullptr;
    TextInput* m_inputer = nullptr;
    Slider* m_slider = nullptr;
    CCMenuItemSpriteExtra* m_btnLeft = nullptr;
    CCMenuItemSpriteExtra* m_btnRight = nullptr;
    // update value from text input
    void textChanged(CCTextInputNode* p) override;
    // check value > max case
    void textInputClosed(CCTextInputNode* p) override;
    // change chroma frequency by slider
    void onSlider(CCObject*);
    // on arrow
    void onArrow(CCObject*);
    void sliderBegan(Slider *p) override {
        SignalEvent("drag-slider", true).post();
    };
    //void sliderBegan(Slider *p) {log::info("slider began");};
    void sliderEnded(Slider *p) override {
        SignalEvent("drag-slider", false).post();
        postEvent();
    };
    // value -> slider
    virtual float Val2Slider(float value) {
        return value > 1 ? 1 : value;
    }
    // slider -> value
    virtual float Slider2Val(float value) {
        return value;
    }
    // post event
    virtual void postEvent() = 0;

    float getVal() {
        return this->value;
    }
    void setVal(float value) {
        this->value = value;
        m_inputer->setString(cocos2d::CCString::createWithFormat("%.2f", static_cast<float>(value))->getCString());
        m_slider->setValue(Val2Slider(value));
    }
};

// speed slider in main menu
class SpeedSliderBundle : public SliderBundleBase {
protected:
    bool init(CCPoint pos, const char* name, float val, float max, int tag, std::string id);
    float Val2Slider(float value) override {
        return sqrt(value/5) > 1 ? 1 : round(sqrt(value/5) * 100) / 100;
    }
    float Slider2Val(float s) override {
        return 5 * s * s;
    }
    void postEvent() override {
        Mod::get()->setSavedValue("speed", this->value);
        SignalEvent<float>("speed", this->value).post();
    }
public:
    void Fade(bool in);

    static SpeedSliderBundle* create(float val) {
        auto node = new SpeedSliderBundle();
        if (node && node->init(CCPoint(0, -120.f), "Frequency", val, -1, 10, "speed-menu")) {
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
class SetupOptionLine : public CCMenu, public TextInputDelegate, public SliderDelegate {
protected:
    // value
    int value = 0;
    // max value, -1 if up to infinity
    float max = 0;
    // init
    bool init(OptionLineType type, int mode, int tag);
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
    // update value from text input
    void textChanged(CCTextInputNode* p) override;
    // check value > max case
    void textInputClosed(CCTextInputNode* p) override;
    // signal
    void sliderBegan(Slider *p) override {
        SignalEvent("drag-slider", true).post();
    };
    // change chroma frequency by slider
    void sliderEnded(Slider *p) override {
        SignalEvent("drag-slider", false).post();
        SignalEvent(mode == 3 ? "duty" :"satu", value).post();
    };
    void onSlider(CCObject*);
    // on arrow
    void onArrow(CCObject*);
    // value -> slider
    float Val2Slider(int value) {
        return (float)value / 100;
    }
    // slider -> value
    int Slider2Val(float value) {
        return (int)round(value * 100);
    }
public:
    OptionLineType type;
    int mode;
    // UI
    CCLabelBMFont* m_label = nullptr;
    CCLabelBMFont* m_title = nullptr;
    CCMenuItemToggler* m_toggler = nullptr;
    CCMenuItemSpriteExtra* m_colpk = nullptr;
    CCMenuItemSpriteExtra* m_colpk1 = nullptr;
    CCMenuItemSpriteExtra* m_colpk2 = nullptr;    
    TextInput* m_inputer = nullptr;
    Slider* m_slider = nullptr;
    CCMenuItemSpriteExtra* m_btnLeft = nullptr;
    CCMenuItemSpriteExtra* m_btnRight = nullptr;
    // toggle the toggler and set/tint the label' color
    void toggleTitle(bool yes, bool fade = false) {
        if (m_toggler)
            m_toggler->toggle(yes && !fade);
        if (type == OptionLineType::Title && m_title)
            // green or gray
            m_title->runAction(CCTintTo::create(fade*ANIM_TIME_M, 127-127*yes, 127+128*yes, 127-127*yes));
    }
    // i hate cascading opacity
    void helpFade(bool in);
    int getVal() {
        return this->value;
    }
    void setVal(int value) {
        this->value = value;
        if (m_inputer)
            m_inputer->setString(cocos2d::CCString::createWithFormat("%i", value)->getCString());
        if (m_slider)
            m_slider->setValue(Val2Slider(value));
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
    // cell actions array
    std::vector<CCAction*> actions;
    // Y offset when fade in
    float offset = 30.f;
    // get a mysterious arg related to animaiton
    float getSomething(float Y, float H);
    void setPosition(CCPoint const& pos) override {
        CCLayerColor::setPosition(pos);

        for (auto child : CCArrayExt<CCMenu*>(m_pChildren)) {
            float tg = getSomething(child->getPositionY(), child->getContentHeight());
            child->setVisible(tg > 0);
            child->setOpacity(255 * tg);
            child->setScale(0.5 + tg/2);
        }
    }
    void addChild(CCNode* child) override {
        CCNode::addChild(child);
        Ystd.push_back(child->getPositionY());
        actions.push_back(nullptr);
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
    void Transition(bool in, bool scale = true);

    static ScrollLayerPlus* create(CCRect const& rect, bool scroll = true, bool vertical = true) {
        auto ret = new ScrollLayerPlus(rect, scroll, vertical);
        ret->autorelease();
        return ret;
    }
};

