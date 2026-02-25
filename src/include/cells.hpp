// A cell class is an inherited CCMenu along with a CCScale9Sprite
// This file includes all self-defined cell class
#pragma once

#include "mynode.hpp"

// This class works as a CCNode with a new CCScale9Sprite background
class BaseCell : public CCMenu {
protected:
    NineSlice* m_bg;
    // setup
    bool setup(CCPoint point, CCSize size, int tag, std::string id);
public:

    // ~by Anal Walker~
    virtual void Fade(bool in) {
        fade(this, in, ANIM_TIME_L);
    }
    // tint
    void tint(float d, int r, int g, int b);
    inline void switchTheme() {
        this->m_bg->runAction(CCTintTo::create(ANIM_TIME_M, CELL_COLOR));
    }
};

// notice player when they launch it in extreme demons
class WarnCell : public BaseCell {
protected:
    CCLabelBMFont* m_title;
    MDTextArea* m_text;
    CCMenuItemSpriteExtra* m_btnConfirm;
    CCMenuItemSpriteExtra* m_btnRemind;
    CCMenuItemSpriteExtra* m_btnEscape;

    bool init() override;
    void onClick(CCObject* sender) {
        Signal<int>("warning").send(sender->getTag());
    }
public:
    void Fade(bool in) override {
        BaseCell::Fade(in);
        fade(m_text, in, ANIM_TIME_L, 1.2 * in, 1.2 * in);

    }

    static WarnCell* create() {
        auto node = new WarnCell();
        if (node && node->init()) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;  
    }
};

// preserved unused class
class TitleCell : public BaseCell {
protected:
    CCClippingNode* m_clip;
    CCSprite* m_spr;
    CCLabelBMFont* m_title;
    CCLayerGradient* m_gradient;
    CCPoint display_pos;
    bool init(const char* text, CCPoint pos, float width, int tag, std::string id);
public:
    void Fade(bool) override;

    static TitleCell* create(const char* title, CCPoint pos, float width, int tag, std::string id) {
        auto node = new TitleCell();
        if (node && node->init(title, pos, width, tag, id)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

// Option Title
class OptionTitleCell : public BaseCell {
protected:
    CCLabelBMFont* m_title;
    // Option Title
    bool init(const char* text, float y, int tag, std::string id);
public:
    static OptionTitleCell* create(const char* title, float y, int tag, std::string id) {
        auto node = new OptionTitleCell();
        if (node && node->init(title, y, tag, id)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

// Option Toggler
class OptionTogglerCell : public BaseCell {
protected:
    bool yes;
    CCMenuItemToggler* m_toggler;
    CCLabelBMFont* m_label;
    CCLabelBMFont* m_hint;

    // Option Toggle
    bool init(const char* title, float y, int tag, std::string id, const char* desc);
public:
    // for switch animation
    void onOption(CCObject*);
    void Fade(bool) override;
    
    static OptionTogglerCell* create(const char* title, float y, int tag, std::string id, const char* desc) {
        auto node = new OptionTogglerCell();
        if (node && node->init(title, y, tag, id, desc)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

/*
class OptionSliderCell : public BaseCell {
protected:
    SliderBundleBase* bundle;
    // Option Float
    bool init(const char* title, float y, int tag, std::string id, const char* desc);
public:
    void Fade(bool) override;
    
    static OptionSliderCell* create(const char* title, float y, int tag, std::string id, const char* desc) {
        auto node = new OptionSliderCell();
        if (node && node->init(title, y, tag, id, desc)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};*/

// This cell works for item menu as a bunch of items
class ItemCell : public BaseCell {
protected:
    // buttons
    std::vector<PickItemButton*> btns;
    // tag = 1/2/3 -> adv/easy/effect
    bool init(int tag);
    void onPickItem(CCObject* sender) {
        Signal<int>("pick").send(sender->getTag());
    }
public:
    void Fade(bool) override;
    void runChroma(float const& phase, float const& percentage, int const& progress) {
        for (auto btn : this->btns)
            btn->runChroma(phase, percentage, progress);
    }
    void toggleChroma() {
        for (auto btn : this->btns)
            btn->toggleChroma();
    }
    void switchPlayer() {
        for (auto btn : this->btns)
            btn->switchPlayer();
    }
    void setModeTarget(Gamemode gamemode) {
        // refresh item menu target
        for (auto btn : this->btns)
            btn->setModeTarget(gamemode);
    }
    static ItemCell* create(int tag) {
        auto node = new ItemCell();
        if (node && node->init(tag)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

// This cell works as a in setup menu left part
class SetupItemCell : public BaseCell {
protected:
    // just his button's tag
    // 0-9 -> icon 10~15 -> effect
    int index;
    // button
    CCLabelBMFont* m_label = nullptr;
    // label
    PickItemButton* m_btn = nullptr;    
    // init
    bool init(int index, float Y, int tag);
    // click the button
    void onPickItem(CCObject* sender) {
        Signal<int>("pick").send(this->index);
    }
public:
    void switchPlayer() {
        this->m_btn->switchPlayer();
    }
    void toggleChroma(bool on) {
        this->m_btn->toggleChroma(on);
    }
    void runChroma(float phase, float percentage, int progress) {
        this->m_btn->runChroma(phase, percentage, progress);
    }
    void select(bool current) {
        // stop chroma
        this->m_btn->toggleChroma(current);
        // tint gray
        this->m_label->runAction(CCEaseExponentialOut::create(
            current ? CCTintTo::create(ANIM_TIME_M, 0, 255, 0) : CCTintTo::create(ANIM_TIME_M, 127, 127, 127)));
        if (current)
            this->tint(ANIM_TIME_M, 0, 80, 0);
        else
            // tint bg
            this->switchTheme();
    }
    bool setModeTarget(Gamemode gamemode) {
        if (index > 9)
            this->m_btn->setModeTarget(gamemode);
        return index > 9;
    }
    static SetupItemCell* create(int index, float Y, int tag) {
        auto node = new SetupItemCell();
        if (node && node->init(index, Y, tag)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

class SetupOptionCell : public BaseCell {
protected:
    //std::string key;
    ChromaSetup setup;
    // i hate cascading his opacity
    //SetupOptionLine* sliderLine = nullptr;
    bool init();
public:
    CCArrayExt<SetupOptionLine*> m_aryMenus;
    // refresh ui status when sth happens. fade = true if needs animation
    void refreshUI(ChromaSetup setup, bool fade = false);
    void Fade(bool in, int dir = 0);
    static SetupOptionCell* create() {
        auto node = new SetupOptionCell();
        if (node && node->init()) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

class ColorValueCell : public BaseCell, public TextInputDelegate, public SliderDelegate {
protected:
    // value
    int value = 255;
    // rgb = 012
    int type;
    CCLabelBMFont* m_label;
    Slider* m_slider;
    TextInput* m_inputer;
    // init
    bool init(int type);
    // update value from text input
    void textChanged(CCTextInputNode* p) override;
    // check value > max case
    void textInputClosed(CCTextInputNode* p) override;
    // change chroma frequency by slider
    inline void onSlider(CCObject*);
    void sliderBegan(Slider *p) override;
    void sliderEnded(Slider *p) override;
    // value -> slider
    inline virtual float Val2Slider(int val) {
        if (val > 255) return 1;
        if (val < 0) return 0;
        return (float)val / 255;
    }
    // slider -> value
    inline virtual int Slider2Val(float s) {
        return round(s * 255);
    }
public:
    void Fade(bool in) override;
    inline int getVal() {
        return value;
    }
    inline void setVal(int value) {
        this->value = value;
        m_inputer->setString(cocos2d::CCString::createWithFormat("%i", value)->getCString());
        m_slider->setValue(Val2Slider(value));
    }
    static ColorValueCell* create(int type) {
        auto node = new ColorValueCell();
        if (node && node->init(type)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

class ColorHexCell : public BaseCell, public TextInputDelegate {
protected:
    std::string str = "FFFFFF";
    CCLabelBMFont* m_label;
    Slider* m_slider;
    TextInput* m_inputer;
    // init
    bool init() override;
    // update value from text input
    void textChanged(CCTextInputNode* p) override;
public:
    void Fade(bool in) override;
    void setColorValue(ccColor3B color) {
        this->str = cc3bToHexString(color);
        m_inputer->setString(str);
    }
    static ColorHexCell* create() {
        auto node = new ColorHexCell();
        if (node && node->init()) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};