// A cell class is an inherited CCMenu along with a CCScale9Sprite
// This file includes all self-defined cell class
#pragma once

#include "mynode.hpp"

// This class works as a CCNode with a CCScale9Sprite background
class BaseCell : public CCMenu {
protected:
    bool setup(CCPoint point, CCSize size, int tag, std::string id);

public:
    CCScale9Sprite* m_bg;
    // ~by Anal Walker~
    virtual void Fade(bool);
    // switch theme
};

// notice player when they launch it in extreme demons
class WarnCell : public BaseCell {
protected:
    CCLabelBMFont* m_title;
    TextArea* m_text;
    CCMenuItemSpriteExtra* m_btnConfirm;
    CCMenuItemSpriteExtra* m_btnRemind;
    CCMenuItemSpriteExtra* m_btnEscape;

    bool init() override;
    void onClick(CCObject* sender) {
        SignalEvent("warning", sender->getTag()).post();
    }
public:
    void Fade(bool in) override {
        BaseCell::Fade(in);
        fade(m_text, in);
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

class TitleCell : public BaseCell {
protected:
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
    bool init(const char* text, float y, float width, int tag, std::string id);
public:
    static OptionTitleCell* create(const char* title, float y, float width, int tag, std::string id) {
        auto node = new OptionTitleCell();
        if (node && node->init(title, y, width, tag, id)) {
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
    bool init(const char* title, float y, float width, bool defaultVal, int tag, std::string id, const char* desc);
public:
    // for switch animation
    void onOption(CCObject*);
    void Fade(bool) override;
    
    static OptionTogglerCell* create(const char* title, float y, float width, bool defaultVal, int tag, std::string id, const char* desc) {
        auto node = new OptionTogglerCell();
        if (node && node->init(title, y, width, defaultVal, tag, id, desc)) {
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
    // tag = 6~9: p1i p1e p2i p2e
    bool init(int tag);
    void onPickItem(CCObject* sender) {
        SignalEvent("pick", sender->getTag()).post();
    }
public:
    bool p2 = false;
    CCArrayExt<PickItemButton*> btns;
    void Fade(bool) override;
    void switchPlayer();
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
    bool p2 = false;
    int tag = 0;
    CCSprite* m_spr = nullptr;
    PickItemButton* m_btn = nullptr;

    bool init(int tag, float Y, int nodeTag);
    void onPickItem(CCObject* sender) {
        SignalEvent("pick", sender->getTag()).post();
    }
public:
    CCLabelBMFont* m_label = nullptr;
    static SetupItemCell* create(int tag, float Y, int nodeTag) {
        auto node = new SetupItemCell();
        if (node && node->init(tag, Y, nodeTag)) {
            node->autorelease();
            return node;
        };
        CC_SAFE_DELETE(node);
        return nullptr;
    }
};

class SetupOptionCell : public BaseCell {
protected:
    //int id;
    //Channel chnl;
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
    void onSlider(CCObject*);
    void sliderBegan(Slider *p) override;
    void sliderEnded(Slider *p) override;
    // value -> slider
    virtual float Val2Slider(int val) {
        if (val > 255) return 1;
        if (val < 0) return 0;
        return (float)val / 255;
    }
    // slider -> value
    virtual int Slider2Val(float s) {
        return round(s * 255);
    }
public:
    void Fade(bool in) override;
    int getVal() {
        return value;
    }
    void setVal(int value) {
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