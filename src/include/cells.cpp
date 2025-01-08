#include "cells.hpp"

extern std::map<std::string, bool> opts;

bool BaseCell::setup(CCPoint point, CCSize size, int tag, std::string id) {
    // myself
    this->setPosition(point);
    this->setContentSize(size);    
    this->setAnchorPoint(CCPoint(0.5, 0.5));
    this->ignoreAnchorPointForPosition(false);
    this->setTag(tag);
    this->setID(id);

    // bg
    m_bg = CCScale9Sprite::create("square.png"_spr);
    m_bg->setPosition(CCPoint(size.width/2, size.height/2));
    m_bg->setContentSize(CCSize(size.width+10, size.height+10));
    m_bg->setZOrder(-1);
    m_bg->setColor(ccc3(CELL_COLOR));
    m_bg->setID("background");
    addChild(m_bg);

    return true;
}

void BaseCell::tint(float d, int r, int g, int b) {
    if (opts["dark-theme"])
        this->m_bg->runAction(CCTintTo::create(ANIM_TIME_M, 255 - (b + g) / 2, 255 - (b + r) / 2, 255 - (r + g) / 2));
    else
        this->m_bg->runAction(CCTintTo::create(ANIM_TIME_M, r, g, b));
}

bool WarnCell::init() {
    if (!CCMenu::init())
        return false;
    
    if (!BaseCell::setup(CCPoint(0.f, 0.f), CCSize(420.f, 280.f), 30, "notice-frame"))
        return false;

    this->setVisible(false);
    this->setScale(0.5);
    this->setOpacity(0.f);

    m_title = CCLabelBMFont::create("WARNING", "ErasBold.fnt"_spr, 240.f, CCTextAlignment::kCCTextAlignmentCenter);
    m_title->setPosition(CCPoint(210.f, 260.f));
    m_title->setScale(0.8);
    this->addChild(m_title);

    m_text = TextArea::create(
        "       It seems like you are playing an extreme demon, perhaps even a list demon."
        " Grateful for your enjoying this mod though, I have to remind you:\n"
        "       Regarding Pointercrate or some other lists' policy, <cy>despite time-varient player color may be allowed,"
        " changing the player color in halfway run of a list demon</c> <cr>WILL PROBABLY GET REJECTED</c> <cy>for submission.</c>\n"
        "       If list points is one of the reasons you are right here, are you still gonna access Chroma Menu now?",
        "ErasWhite.fnt"_spr, 0.5, 500.f, CCPoint(0.f, 0.5), 18.f, false);
    m_text->setPosition(CCPoint(218.f, 190.f));
    m_text->setContentSize(CCSize(400.f, 140.f));
    // wtf here
    int index = 0;
    for (auto obj: CCArrayExt<CCLabelBMFont*>(m_text->m_label->m_lines)) {
        if (index < 2)
            obj->setPositionY(obj->getPositionY() + 6.f);
        if (index < 5)
            obj->setPositionY(obj->getPositionY() + 6.f);
        index ++;
    }
    this->addChild(m_text);

    auto labConfirm = CCLabelBMFont::create("YES ALWAYS", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    labConfirm->setScale(0.4);
    m_btnConfirm = CCMenuItemSpriteExtra::create(labConfirm, this, menu_selector(WarnCell::onClick));
    m_btnConfirm->setPosition(CCPoint(210.f, 90.f));
    //m_btnConfirm->setColor(ccc3(255-col, 255-col, 255-col));
    m_btnConfirm->setTag(2);
    this->addChild(m_btnConfirm);

    auto hintConfirm = CCLabelBMFont::create(
        "I know what this mod means to record submission, never pop it again.",
        "ErasLight.fnt"_spr, 400.f, CCTextAlignment::kCCTextAlignmentCenter);
    hintConfirm->setPosition(CCPoint(210.f, 78.f));
    hintConfirm->setColor(ccc3(255, 255, 0));
    hintConfirm->setScale(0.6);
    this->addChild(hintConfirm);

    auto labRemind = CCLabelBMFont::create("YES THIS TIME", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    labRemind->setScale(0.4);
    m_btnRemind = CCMenuItemSpriteExtra::create(labRemind, this, menu_selector(WarnCell::onClick));
    m_btnRemind->setPosition(CCPoint(210.f, 55.f));
    //m_btnRemind->setColor(ccc3(255-col, 255-col, 255-col));
    m_btnRemind->setTag(1);
    this->addChild(m_btnRemind);

    auto hintRemind = CCLabelBMFont::create(
        "Yes for now but please still remind me next time.",
        "ErasLight.fnt"_spr, 400.f, CCTextAlignment::kCCTextAlignmentCenter);
    hintRemind->setPosition(CCPoint(210.f, 43.f));
    hintRemind->setColor(ccc3(255, 255, 0));
    hintRemind->setScale(0.6);
    this->addChild(hintRemind);

    auto labEscape = CCLabelBMFont::create("EXIT", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    labEscape->setScale(0.4);
    m_btnEscape = CCMenuItemSpriteExtra::create(labEscape, this, menu_selector(WarnCell::onClick));
    m_btnEscape->setPosition(CCPoint(210.f, 20.f));
    m_btnEscape->setColor(ccc3(255, 96, 96));
    m_btnEscape->setTag(0);
    this->addChild(m_btnEscape);

    auto hintEscape = CCLabelBMFont::create(
        "Quit right now and tell the list team I not ever clicked the Chroma Icons button.",
        "ErasLight.fnt"_spr, 400.f, CCTextAlignment::kCCTextAlignmentCenter);
    hintEscape->setPosition(CCPoint(210.f, 8.f));
    hintEscape->setColor(ccc3(255, 255, 0));
    hintEscape->setScale(0.6);
    this->addChild(hintEscape);

    return true;
}

bool TitleCell::init(const char* text, CCPoint pos, float width, int tag, std::string id) {
    if (!CCMenu::init())
        return false;

    m_title = CCLabelBMFont::create(text, "ErasBold.fnt"_spr, 120.f, CCTextAlignment::kCCTextAlignmentCenter);
    m_title->setScale(0.6);
    m_title->setContentSize(CCSize(width, 20.f));
    m_title->setWidth(340.f);
    m_title->setAnchorPoint(CCPoint(0.5, 0.5));
    m_title->setColor(ccc3(255, 255, 255));
    m_title->setID("label");
    addChild(m_title);

    // setup
    if (!BaseCell::setup(pos, CCSize(width, 20.f), tag, id))
        return false;

    m_bg->setVisible(false);
    this->display_pos = pos;
    setTag(tag);
    setID(id);
    // set subnode position
    m_title->setPosition(CCPoint(width/2, 10.f));
    return true;
}

void TitleCell::Fade(bool in) {
    this->BaseCell::Fade(in);
    this->runAction(CCSequence::create(
        CCEaseExponentialOut::create(CCMoveTo::create(ANIM_TIME_M, CCPoint(display_pos.x, display_pos.y - 30.f * in))),
        nullptr
    ));
}

// title option
bool OptionTitleCell::init(const char* text, float y, float width, int tag, std::string id) {
    // parent
    if (!CCMenu::init())
        return false;

    m_title = CCLabelBMFont::create(text, "ErasBold.fnt"_spr, 120.f, CCTextAlignment::kCCTextAlignmentCenter);
    m_title->setScale(0.5);
    m_title->setContentSize(CCSize(width, 20.f));
    m_title->setWidth(340.f);
    m_title->setAnchorPoint(CCPoint(0.5, 0.5));
    m_title->setColor(ccc3(64, 192, 255));
    m_title->setID("title");
    addChild(m_title);

    // setup
    if (!BaseCell::setup(CCPoint(160.f, y-10.f), CCSize(width, 20.f), tag, id))
        return false;

    // set subnode position
    m_title->setPosition(CCPoint(width/2, 10.f));
    return true;
}

// toggler option
bool OptionTogglerCell::init(const char* title, float y, float width, int tag, std::string id, const char* desc) {
    if (!CCMenu::init())
        return false;

    this->yes = Mod::get()->getSavedValue<bool>(id);
    // add hint first to see the height
    m_hint = CCLabelBMFont::create(desc, "ErasLight.fnt"_spr, 120.f, CCTextAlignment::kCCTextAlignmentLeft);
    m_hint->setScale(0.7);
    m_hint->setWidth(width - 10.f);
    m_hint->setAnchorPoint(CCPoint(0.f, 0.f));
    m_hint->setColor({255, 255, 0});
    m_hint->setID("hint");
    addChild(m_hint);

    m_toggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(OptionTogglerCell::onOption), 0.6);
    m_toggler->setCascadeOpacityEnabled(true);
    m_toggler->setID("toggler");
    m_toggler->toggle(yes);
    addChild(m_toggler);

    m_label = CCLabelBMFont::create(title, "ErasBold.fnt"_spr, 120.f, CCTextAlignment::kCCTextAlignmentLeft);
    m_label->setScale(0.45);
    m_label->setContentSize(CCSize(width - 25.f, 20.f));
    m_label->setWidth(340.f);
    m_label->setAnchorPoint(CCPoint(0.f, 0.5));
    m_label->setID("label");
    addChild(m_label);

    // set subnode position
    m_label->setPosition(CCPoint(25.f, 10.f + 0.7 * m_hint->getContentHeight()));
    m_toggler->setPosition(CCPoint(10.f, 10.f + 0.7 * m_hint->getContentHeight()));
    m_hint->setPosition(CCPoint(10.f, 0.f));

    // setup
    if (!BaseCell::setup(CCPoint(160.f, y-10.f + 0.35*m_hint->getContentHeight()), CCSize(300.f, 0.7 * m_hint->getContentHeight() +20.f), tag, id))
        return false;

    // switch!
    if (id == "activate") {
        m_label->setCString(yes ? "Switch : ON" : "Switch : OFF");
        m_label->setColor(ccc3(255-255*yes, 255*yes, 0));
        // tint bg
        this->tint(0, 80 * (!yes), 80 * (yes), 0);
    }
    return true;
}

void OptionTogglerCell::onOption(CCObject* sender) {
    // revert
    yes = !yes;
    // set value
    Mod::get()->setSavedValue(this->getID(), yes);
    // option
    opts[this->getID()] = yes;
    // post signal
    SignalEvent<bool>(this->getID(), yes).post();

    // switch
    if (this->getID() == "activate") {
        m_label->setCString(yes ? "Switch : ON" : "Switch : OFF");
        m_label->setColor(ccc3(255-255*yes, 255*yes, 0));
        // tint bg
        this->tint(ANIM_TIME_M, 80 * (!yes), 80 * (yes), 0);
    }
}

void OptionTogglerCell::Fade(bool in) {
    BaseCell::Fade(in);
    auto btn = this->getChildByType<CCMenuItem>(0);
    if (in)
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_L),
            CallFuncExt::create([btn](void){ btn->setEnabled(true); }),
            nullptr
        ));
    else
        btn->setEnabled(false);
}
bool ItemCell::init(int tag) {
    if (!CCMenu::init())
        return false;

    // effect
    if (tag == 3) {
        // setup
        if (!BaseCell::setup(CCPoint(0.f, -55.f), CCSize(150.f, 24.f), tag, "effect-sheet"))
            return false;

        for (int item = 11; item < 16; item ++) {
            // spr
            auto btn = PickItemButton::create(item, false, this, menu_selector(ItemCell::onPickItem));
            btn->setPosition(CCPoint(30.f*item - 315.f, 12.f));
            btn->setScale(0);
            btn->setOpacity(0);
            this->addChild(btn);
            this->btns.push_back(btn);
        }        
    }
    // easy mode icons
    else if (tag == 2) {
        // setup
        if (!BaseCell::setup(CCPoint(0.f, 20.f), CCSize(50.f, 50.f), tag, "easy-sheet"))
            return false;
        // btn
        auto btn = PickItemButton::create(0, false, this, menu_selector(ItemCell::onPickItem));
        btn->setPosition(CCPoint(25.f, 25.f));
        // init status
        btn->setScale(0);
        btn->setOpacity(0);
        this->addChild(btn);
        this->btns.push_back(btn);
    }
    // advanced icons
    else {
        // setup
        if (!BaseCell::setup(CCPoint(0.f, 20.f), CCSize(400.f, 34.f), tag, "advanced-sheet"))
            return false;
        // full mode icons
        for (int status = 1; status < 10; status ++) {
            // btn
            auto btn = PickItemButton::create(status, false, this, menu_selector(ItemCell::onPickItem));
            btn->setPosition(CCPoint(45.f * status - 25.f, 17.f));
            // init status
            btn->setScale(0);
            btn->setOpacity(0);
            this->addChild(btn);
            this->btns.push_back(btn);
        }
    }
    
    // say goodbye to init
    m_bg->setTag(100);
    m_bg->setScale(5, 0.2);
    m_bg->setOpacity(0);

    return true;
}

void ItemCell::Fade(bool in) {
    // bg
    fade(this->m_bg, in, ANIM_TIME_M, in ? 1 : 5, in ? 1 : 0.2);
    // items
    int length = btns.size();
    for (int i = 0; i < length; i++) {
        btns[i]->delayFade(in ? abs(length/ 2 - i) : 0, in);
    }
}

bool SetupItemCell::init(int index, float Y, int tag) {
    if (!CCMenu::init())
        return false;

    if (!BaseCell::setup(CCPoint(60.f, Y), CCSize(90.f, 24.f), tag, "sheet"))
        return false;

    this->index = index;

    m_label = CCLabelBMFont::create((index > 9 ? chnls[index - 7] : items[index]).c_str(), "ErasBold.fnt"_spr, 120.f, CCTextAlignment::kCCTextAlignmentLeft);
    m_label->setScale(0.4);
    // wave trail label is too long
    if (index == 12)
        m_label->setScaleX(0.35);
    m_label->setPosition(CCPoint(60.f, 12.f));
    m_label->setContentSize(CCSize(60.f, 24.f));
    m_label->setWidth(65.f);
    m_label->setColor(ccc3(127, 127, 127));
    m_label->setID("label");
    this->addChild(m_label);

    // btn
    m_btn = PickItemButton::create(index, true, this, menu_selector(SetupItemCell::onPickItem));
    m_btn->setPosition(CCPoint(14.f, 12.f));
    this->addChild(m_btn);

    this->setCascadeOpacityEnabled(true);
    return true;
}

bool SetupOptionCell::init() {
    if (!BaseCell::setup(CCPoint(50.f, 0.f), CCSize(240.f, 230.f), 0, "setup-options"))
        return false;

    this->setVisible(false);
    this->setOpacity(0.f);
    this->setScale(0.5);
    this->setCascadeOpacityEnabled(true);
    // node ids
    std::vector<std::vector<int>> array = {{1, 2}, {1, 2, 3}, {1, 2, 5}, {1, 2, 4, 5}, {1, 2, 4, 6}};
    int t = 0;
    for (int mode = 0; mode < 5; mode++) {
        for (auto type: array.at(mode)) {
            // tag plus one
            t ++;
            // construct
            auto optionNode = SetupOptionLine::create(OptionLineType(type), mode, t);
            this->addChild(optionNode);
            m_aryMenus.push_back(optionNode);
        }
    }
    return true;
}

void SetupOptionCell::refreshUI(ChromaSetup setup, bool fade) {
    float Y = this->getContentHeight() - 5.f;
    // temply we read the progress like this to avoid crash as possible as i can
    int duty = 180;
    ccColor3B grad1 = setup.gradient.begin()->second;
    ccColor3B grad2 = setup.gradient.rbegin()->second;
    for (auto p : setup.gradient) {
        if (p == *setup.gradient.begin())
            continue;
        // get duty
        duty = p.second == grad2 ? p.first : p.first / 2 + 180;
    }
    for (auto node: m_aryMenus) {
        // title toggle and tint
        if (node->type == OptionLineType::Title) {
            Y -= 5.f;            
            node->toggleTitle(node->mode == setup.mode, fade);
        } else
            switch (node->type) {
            case OptionLineType::SingleColor:
                node->m_colpk->setColor(setup.color);
                break;
            case OptionLineType::MultiColor:
                node->m_colpk1->setColor(node->mode == 4 ? setup.progress.begin()->second : grad1);
                node->m_colpk2->setColor(node->mode == 4 ? (setup.progress.rbegin())->second : grad2);
                break;
            case OptionLineType::Toggler:
                node->m_toggler->toggle(setup.best);
                break;
            case OptionLineType::Slider:
                node->setVal(node->mode == 3 ? (int)(duty / 3.6) : setup.satu);
                break;
            default:
                break;
            }

        bool display = node->type == OptionLineType::Title || node->mode == setup.mode;
        auto action = CCSpawn::create(
            CCEaseExponentialOut::create(CCMoveTo::create(fade*ANIM_TIME_M, CCPoint(this->getContentWidth() / 2, Y))),
            CCEaseExponentialOut::create(CCScaleTo::create(fade*ANIM_TIME_M, 1, display)),
            CCEaseExponentialOut::create(CCFadeTo::create(fade*ANIM_TIME_M, display*255)),
            CallFuncExt::create([node, display](void){ if (node->type == OptionLineType::Slider) node->helpFade(display); }),
            nullptr
        );

        if (display) {
            node->setVisible(true);
            node->runAction(action);
            Y -= node->getContentHeight();
        } else
            node->runAction(CCSequence::create(
                action,
                CallFuncExt::create([node, display](void) { node->setVisible(display); }),
                nullptr
            ));
    }
}

void SetupOptionCell::Fade(bool in, int dir) {
    this->setPosition(CCPoint(50.f + 200 * dir * in, 0.f));
    BaseCell::Fade(in);
    static_cast<SetupOptionLine*>(getChildByID("satu"))->helpFade(in);
    static_cast<SetupOptionLine*>(getChildByID("duty"))->helpFade(in);
    // move
    if (dir)
        this->runAction(CCEaseExponentialOut::create(
            CCMoveTo::create(ANIM_TIME_M, CCPoint(50.f - 200 * dir * (1-in), 0.f))));
}

std::string rgbLabels[3] = {"R", "G", "B"};

bool ColorValueCell::init(int type) {
    if (!CCMenu::init())
        return false;

    this->type = type;
    // label
    this->m_label = CCLabelBMFont::create(rgbLabels[type].c_str(), "ErasBold.fnt"_spr, 15.f, CCTextAlignment::kCCTextAlignmentLeft);
    m_label->setPosition(CCPoint(10.f, 10.f));
    m_label->setAnchorPoint(CCPoint(0.f, 0.5));
    m_label->setColor(ccc3(200 + 55 * (type==0), 200 + 55 * (type==1), 200 + 55 * (type==2)));
    m_label->setScale(0.5);
    this->addChild(m_label);

    // inputer
    this->m_inputer = TextInput::create(50.f, rgbLabels[type].c_str(), "ErasBold.fnt"_spr);
    m_inputer->setPosition(CCPoint(140.f, 10.f));
    m_inputer->setFilter("1234567890");
    m_inputer->setMaxCharCount(3);
    m_inputer->setDelegate(this);
    m_inputer->getChildByType<CCScale9Sprite>(0)->setVisible(false);
    m_inputer->setScale(0.9);
    m_inputer->setID("text-input");
    this->addChild(m_inputer);

    // slider
    this->m_slider = Slider::create(this, menu_selector(ColorValueCell::onSlider), 0.4);
    m_slider->setPosition(CCPoint(70.f, 10.f));
    m_slider->m_delegate = this;
    m_slider->setID("slider");
    this->addChild(m_slider);

    // setup
    if (!BaseCell::setup(CCPoint(130.f, 65.f - 40.f * type), CCSize(160.f, 20.f), type + 1, rgbLabels[type]))
        return false;

    this->setScale(0.5);
    this->setOpacity(0);
    this->setVisible(false);
    return true;
}

void ColorValueCell::textChanged(CCTextInputNode* p) {
    std::string input = p->getString();
    if (input != "") {
        value = stoi(input);
        if (value > 255)
            value = 255;
        m_slider->setValue(Val2Slider(value));
        SignalEvent("color-" + rgbLabels[type], value).post();
    }

}

void ColorValueCell::textInputClosed(CCTextInputNode* p) {
    std::string input = p->getString();
    if (input == "")
        input = std::to_string(value);
    else {
        value = stof(input);
        if (value > 255)
            value = 255;
        input = std::to_string(value);
    }
    p->setString(input);
    SignalEvent("color-" + rgbLabels[type], value).post();
}

void ColorValueCell::onSlider(CCObject* sender) {
    value = Slider2Val(m_slider->getValue());
    m_inputer->setString(std::to_string(value));
    SignalEvent("color-" + rgbLabels[type], value).post();
}
void ColorValueCell::sliderBegan(Slider *p) {
    SignalEvent("drag-slider", true).post();
    // tint bg
    this->tint(ANIM_TIME_M, 50 * (type==0), 50 * (type==1), 50 * (type==2));
}
void ColorValueCell::sliderEnded(Slider *p) {
    SignalEvent("drag-slider", false).post();
    // tint bg
    m_bg->runAction(CCTintTo::create(ANIM_TIME_M, CELL_COLOR));
    SignalEvent("color-" + rgbLabels[type], value).post();
};

void ColorValueCell::Fade(bool in) {
    BaseCell::Fade(in);
    m_slider->getChildByType<CCSprite>(0)->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255*in)));
    m_slider->m_sliderBar->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255*in)));
    m_slider->getThumb()->getChildByTag(1)->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255*in)));

    m_inputer->getChildByType<CCTextInputNode>(0)->getChildByType<CCLabelBMFont>(0)->runAction(
        CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255*in)));     
}

bool ColorHexCell::init() {
    if (!CCMenu::init())
        return false;

    // label
    m_label = CCLabelBMFont::create("HEX", "ErasBold.fnt"_spr, 40.f, CCTextAlignment::kCCTextAlignmentLeft);
    m_label->setPosition(CCPoint(10.f, 10.f));
    m_label->setAnchorPoint(CCPoint(0.f, 0.5));
    m_label->setColor(ccc3(200, 200, 200));
    m_label->setScale(0.5);
    this->addChild(m_label);

    // inputer
    m_inputer = TextInput::create(90.f, "HEX", "ErasBold.fnt"_spr);
    m_inputer->setPosition(CCPoint(95.f, 10.f));
    m_inputer->setFilter("1234567890ABCDEFabcdef");
    m_inputer->setMaxCharCount(6);
    m_inputer->setDelegate(this);
    m_inputer->getChildByType<CCScale9Sprite>(0)->setVisible(false);
    m_inputer->setID("text-input");
    m_inputer->setScale(0.9);
    this->addChild(m_inputer);

    // setup
    if (!BaseCell::setup(CCPoint(130.f, -65.f), CCSize(140.f, 20.f), 0, "HEX"))
        return false;

    this->setScale(0.5);
    this->setOpacity(0);
    this->setVisible(false);
    return true;
}

// update value from text input
void ColorHexCell::textChanged(CCTextInputNode* p) {
    str = p->getString();
    if (auto color = cc3bFromHexString(p->getString(), true))
        SignalEvent("color-hex", color.unwrap()).post();
}

void ColorHexCell::Fade(bool in) {
    BaseCell::Fade(in);
    m_inputer->getChildByType<CCTextInputNode>(0)->getChildByType<CCLabelBMFont>(0)->runAction(
        CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255*in)));     
}