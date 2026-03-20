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
    virtual void Fade(bool in);
    // tint
    void tint(float d, int r, int g, int b);
    // black / white
    virtual void switchTheme();
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
    void onClick(CCObject* sender);
public:
    void Fade(bool in) override;

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
    CCPoint m_pos;
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
    bool m_yes;
    std::string m_title;
    std::string m_desc;
    CCMenuItemToggler* m_toggler;
    CCLabelBMFont* m_label;
    CCMenuItemSpriteExtra* m_hint;

    // Option Toggle
    bool init(std::string title, float y, int tag, std::string id, std::string desc);
public:
    // for switch animation
    void onOption(CCObject*);
    // for description
    void onDesc(CCObject*);
    // switch theme
    void switchTheme();
    // create
    static OptionTogglerCell* create(const char* title, float y, int tag, std::string id, std::string desc) {
        auto node = new OptionTogglerCell();
        if (node && node->init(title, y, tag, id, desc)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

class OptionArrowCell : public BaseCell {
protected:
    // not end value
    int m_index;
    std::vector<int> m_enums;
    std::string m_title;
    std::string m_desc;
    CCMenuItemSpriteExtra* m_btnArrowL;
    CCMenuItemSpriteExtra* m_btnArrowR;
    CCLabelBMFont* m_label, * m_display;
    CCMenuItemSpriteExtra* m_hint;
    // stored index -> displayed string
    std::function<std::string (int)> getReal;
    // Step Toggle
    bool init(std::string title, float y, int tag, std::string id, std::string desc, std::vector<int> enums, std::function<std::string (int)> getReal);
    void onArrow(CCObject* sender);
    void onDesc(CCObject* sender);
    // switch theme
    void switchTheme();
public:
    static OptionArrowCell* create(const char* title, float y, int tag, std::string id, std::string desc, std::vector<int> enums, std::function<std::string (int)> getReal) {
        auto node = new OptionArrowCell();
        if (node && node->init(title, y, tag, id, desc, enums, getReal)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

class OptionSliderCell : public BaseCell, public SliderDelegate {
protected:
    float m_value;
    float m_min;
    float m_max;
    float m_precision;

    std::string m_title;
    std::string m_desc;

    CCLabelBMFont* m_label, * m_display;
    Slider* m_slider;
    CCMenuItemSpriteExtra* m_hint;

    // value to slider (this may return a value out of 1~0 range, unfiltered)
    std::function<float (float)> m_toSlider;
    // slider to value
    std::function<float (float)> m_fromSlider;

    bool init(const char* title, float y, int tag, std::string id, std::string desc, float min, float max, int precision,
        std::function<float (float)> toSlider, std::function<float (float)> fromSlider);

    void sliderBegan(Slider* slider) override;
    void sliderEnded(Slider* slider) override;    

    void onDesc(CCObject* sender);
    void onSlider(CCObject* sender);
    void postEvent();
public:
    static OptionSliderCell* create(const char* title, float y, int tag, std::string id, std::string desc, float min, float max, int precision,
        std::function<float (float)> toSlider, std::function<float (float)> fromSlider) {
        auto node = new OptionSliderCell();
        if (node && node->init(title, y, tag, id, desc, min, max, precision, toSlider, fromSlider)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

// This cell works for item menu as a bunch of items
class ItemCell : public BaseCell {
protected:
    // buttons
    std::vector<PickItemButton*> m_btns;
    // tag = 1/2/3 -> adv/easy/effect
    bool init(int tag);
    void onPickItem(CCObject* sender);
public:
    void Fade(bool) override;
    void runChroma(float const& phase, float const& percentage, int const& progress);
    void toggleChroma();
    void switchPlayer();
    void setModeTarget(Gamemode gamemode);
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
    int m_index;
    // button
    CCLabelBMFont* m_label = nullptr;
    // label
    PickItemButton* m_btn = nullptr;    
    // init
    bool init(int index, float Y, int tag);
    // click the button
    void onPickItem(CCObject* sender);
public:
    void switchPlayer();
    void toggleChroma(bool on);
    void runChroma(float phase, float percentage, int progress);
    void select(bool current);
    bool setModeTarget(Gamemode gamemode);
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
    ChromaPattern m_setup;
    // i hate cascading his opacity
    bool init();
public:
    CCArrayExt<SetupOptionLine*> m_aryMenus;
    // refresh ui status when sth happens. fade = true if needs animation
    void refreshUI(ChromaPattern setup, bool fade = false);
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
    int m_value = 255;
    // rgb = 012
    int m_type;
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
    virtual float Val2Slider(int val);
    // slider -> value
    virtual int Slider2Val(float s);
public:
    void Fade(bool in) override;
    int getVal();
    void setVal(int value);
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
    std::string m_str = "FFFFFF";
    CCLabelBMFont* m_label;
    Slider* m_slider;
    TextInput* m_inputer;
    // init
    bool init() override;
    // update value from text input
    void textChanged(CCTextInputNode* p) override;
public:
    void Fade(bool in) override;
    void setColorValue(ccColor3B const& color);

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