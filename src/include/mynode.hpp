// This file includes all basic inherited ui node classes
#pragma once

#include "utility.hpp"
#include <Geode/binding_arm/CCMenuItemSpriteExtra.hpp>
#include <Geode/ui/TextInput.hpp>

// get child by index and cast to type
template<typename T>
inline T getChildByIndex(CCNode *node, int index) {
    return static_cast<T*>(node->getChildren()->objectAtIndex(index));
}

/********** UI ***********/

class GJItemEffect : public CCSprite {
protected:
    // register effectType
    Channel m_channel;
    // the item in mod menu as this target gamemode
    Gamemode m_gamemode;
    // covering node sprite showing it's reference
    CCSprite* m_cover;
    // init
    bool init(int tab);
public:
    Channel getChannel() { return this->m_channel; }
    void setChannel(Channel channel) { this->m_channel = channel; }
    Gamemode getGamemode() { return this->m_gamemode; }
    void setGamemode(Gamemode gamemode) { this->m_gamemode = gamemode; }
    CCSprite* cover() { return this->m_cover; }

    // create
    // @param tab the tab int this item is created for
    static GJItemEffect* createEffectItem(int tab) {
        auto node = new GJItemEffect();
        if (node && node->init(tab)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

class PickItemButton : public CCMenuItemSpriteExtra {
protected:
    // this icon is from setup page or not
    bool m_src;
    // is player 2
    bool m_ptwo;
    // should be chromatic
    bool m_chroma;
    // icon sprite (gamemode tab only)
    GJItemIcon* m_icon;
    // player (gamemode tab only)
    SimplePlayer* m_player;
    // effect sprite (effect tab only)
    GJItemEffect* m_effect;
    // default colors
    ccColor3B m_mainColor;
    ccColor3B m_secondColor;
    ccColor3B m_glowColor;
    // init
    bool init(int tag, bool src, CCObject* target, cocos2d::SEL_MenuHandler callback);
public:
    // set frame and default colors
    void switchPlayer();
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
    void setModeTarget(Gamemode gamemode);
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

// just slider, preserved
/*
class SliderLayer : public CCLayer {
protected:
    // normalized 0-1
    float value;
    // size
    CCSize size;
    // base and indicator
    CCSprite* base, * target;
    // init
    bool init(CCSize size);
    // decide it's touched or not
    bool ccTouchBegan(CCTouch *touch, CCEvent* event) override;
    // move touch to update progress
    void ccTouchMoved(CCTouch *touch, CCEvent* event) override;
public:
    // set value
    void setValue(float value);
    // create
    static SliderLayer* create(CCSize size) {
        auto node = new SliderLayer();
        if (node && node->init(size)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};*/

// a node with feedback logic design of a slider + input
class SliderBundleBase : public CCMenu, public TextInputDelegate, public SliderDelegate {
protected:
    // topic
    std::string m_topic;
    // value
    float m_value;
    // max value, -1 if up to infinity
    float m_max;
    // min value
    float m_min;
    // precision
    float m_precision;
    // label
    CCLabelBMFont* m_label;
    // inputer
    TextInput* m_inputer;
    // slider
    Slider* m_slider;
    // left arrow
    CCMenuItemSpriteExtra* m_btnLeft;
    // right arrow
    CCMenuItemSpriteExtra* m_btnRight;  
    // value to slider (this may return a value out of 1~0 range, unfiltered)
    std::function<float (float)> m_toSlider;
    // slider to value
    std::function<float (float)> m_fromSlider;
    // worst init function ever
    bool init(std::string topic, const char* title, float value, float max, float min, int precision, bool has_arrow, \
        float labelScale,  float sliderScale, float inputerScale, float arrowScale, float sliderPosX, float inputerPosX, float labelWidth, float inputerWidth, float arrowDistance,\
        std::function<float (float)> toSlider, std::function<float (float)> fromSlider);
public:
    // update value from text input
    void textChanged(CCTextInputNode* p) override {
        std::string input = p->getString();
        this->setVal(std::clamp(numFromString<float>(input, this->m_precision).unwrapOr(this->m_value), 0.f, 1.f));
    }
    // check value > max case
    void textInputClosed(CCTextInputNode* p) override {
        std::string input = p->getString();
        this->setVal(std::clamp(numFromString<float>(input, this->m_precision).unwrapOr(this->m_value), this->m_min, this->m_max));
        this->postEvent();
    }
    // change chroma frequency by slider
    void onSlider(CCObject* sender) {
        this->setVal(this->m_fromSlider(this->m_slider->getValue()), -1);
    }
    // on arrow
    void onArrow(CCObject* sender) {
        float delta = sender->getTag() == 2 ? 0.1 : -0.1;
        float news = std::clamp(this->m_toSlider(this->m_value) + delta, 0.f, 1.f);
        this->setVal(this->m_fromSlider(news));
        this->postEvent();
    }
    // mute onClose
    void sliderBegan(Slider *p) override {
        Signal<bool>("drag-slider").send(true);
    };
    // unmute onClose and post event
    void sliderEnded(Slider *p) override {
        Signal<bool>("drag-slider").send(false);
        this->setVal(this->m_fromSlider(this->m_slider->getValue()));
        this->postEvent();
    };
    // post event
    virtual void postEvent() {
        if (this->m_precision)
            Signal<float>(this->m_topic).send(this->m_value);
        else
            Signal<int>(this->m_topic).send((int)this->m_value);
    }
    float getVal() { return this->m_value; }
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
    Color = 3,
    MultiColor = 4,
    Slider = 5,
    Toggler = 6
};

// Setup Option Line, not a cell
class SetupOptionLine : public SliderBundleBase {
protected:
    // hint (title only)
    CCMenuItemSpriteExtra* m_hint;
    // init
    bool init(OptionLineType type, int mode, int tag);
    // toggler callback
    void onToggle(CCObject*) {
        // mode
        if (this->m_type == OptionLineType::Title)
            Signal<int>("mode").send(this->m_mode);
        // best toggler
        else if (this->m_type == OptionLineType::Toggler)
            Signal<bool>("best").send(!this->m_toggler->isToggled());
    };
    void onPickColor(CCObject* sender) {
        Signal<int>("color").send(sender->getTag());
    };
    // on desc
    void onDesc(CCObject* sender);
public:
    // line ui type
    OptionLineType m_type;
    // the mode this line points to
    int m_mode;
    // title (title line only)
    CCLabelBMFont* m_title;
    // toggler (shared)
    CCMenuItemToggler* m_toggler;
    // color picker
    CCMenuItemSpriteExtra* m_colpk;
    // color picker1
    CCMenuItemSpriteExtra* m_colpk1;
    // color picker2
    CCMenuItemSpriteExtra* m_colpk2;
    // toggle the toggler and set/tint the label' color
    void toggleTitle(bool yes, bool fade = false);

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
    float m_ceilingHeight = 0;
    // nodes' Y positions when they are on display
    // arranged by tag order, from bottom to top
    std::vector<float> m_Ystd;
    // node action volume
    std::vector<CCAction*> m_acts;
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
        this->m_Ystd.push_back(child->getPositionY());
        this->m_acts.push_back(nullptr);
    }

    static MyContentLayer* create(float width, float height) {
        auto ret = new MyContentLayer();
        if (ret->initWithColor(ccc4(0, 0, 0, 0 ), width, height)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class ScrollLayerPlus : public ScrollLayer {
protected:
    CCAction* m_actionFade = nullptr;
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
        static_cast<MyContentLayer*>(this->m_contentLayer)->m_ceilingHeight = getContentHeight();
    }
    void Transition(bool in, int move);

    static ScrollLayerPlus* create(CCRect const& rect, bool scroll = true, bool vertical = true) {
        auto ret = new ScrollLayerPlus(rect, scroll, vertical);
        ret->autorelease();
        return ret;
    }
};

