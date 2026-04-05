#include "cells.hpp"

extern std::map<std::string, bool> opts;
extern std::map<std::string, float> vals;

bool BaseCell::setup(CCPoint point, CCSize size, int tag, std::string id) {
    // myself
    this->setPosition(point);
    this->setContentSize(size);
    this->setAnchorPoint(CCPoint(0.5, 0.5));
    this->ignoreAnchorPointForPosition(false);
    this->setTag(tag);
    this->setID(id);

    // bg
    this->m_bg = NineSlice::create("square.png"_spr);
    this->m_bg->setPosition(CCPoint(size.width/2, size.height/2));
    this->m_bg->setContentSize(CCSize(size.width+10, size.height+10));
    this->m_bg->setZOrder(-1);
    this->m_bg->setColor(ccc3(CELL_COLOR));
    this->m_bg->setID("background");
    this->addChild(this->m_bg);

    return true;
}

void BaseCell::Fade(bool in) {
    fade(this, in, ANIM_TIME_L);
}

void BaseCell::switchTheme() {
    this->m_bg->runAction(CCTintTo::create(ANIM_TIME_M, CELL_COLOR));
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

    this->m_title = CCLabelBMFont::create("WARNING", "ErasBold.fnt"_spr, 240.f, CCTextAlignment::kCCTextAlignmentCenter);
    this->m_title->setPosition(CCPoint(210.f, 260.f));
    this->m_title->setScale(0.8);
    this->addChild(this->m_title);

    this->m_text = MDTextArea::create(
        "- It looks like your are playing an extreme demon, perhaps also a list demon (Chroma Icons doesn't access Pointercrate so idk)."
        "Grateful for your enjoyiing this mod though, I should remind you:\n"
        "- Regarding Pointercrate and some other lists' guidelines and policies, despite time-varient player colors may be allowed sometimes,"
        "<cr>quite a lot</c> behaviors of this mod may break the lists' guidelines for submission, <cr>especially you editing chroma patterns in halfway beating a demon;</c>"
        "Those will likely result in your submission getting <cr>**REJECTED**</c>.\n"
        "- Considering list points might be one of the reasons you are right here ~~or even the only~~, are you still gonna go ahead, my friend?",
        ccp(333.f, 120.f)
    );

    this->m_text->setPosition(CCPoint(210.f, 170.f));
    this->m_text->setScale(1.2);
    this->m_text->getScrollLayer()->m_disableMovement = true;
    this->addChild(this->m_text);

    auto labConfirm = CCLabelBMFont::create("YES ALWAYS", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    labConfirm->setScale(0.4);
    this->m_btnConfirm = CCMenuItemSpriteExtra::create(labConfirm, this, menu_selector(WarnCell::onClick));
    this->m_btnConfirm->setPosition(CCPoint(210.f, 84.f));
    //m_btnConfirm->setColor(ccc3(255-col, 255-col, 255-col));
    this->m_btnConfirm->setTag(2);
    this->addChild(this->m_btnConfirm);

    auto hintConfirm = CCLabelBMFont::create(
        "I know what this mod means to record submission, never pop it again.",
        "ErasLight.fnt"_spr, 400.f, CCTextAlignment::kCCTextAlignmentCenter);
    hintConfirm->setPosition(CCPoint(210.f, 72.f));
    hintConfirm->setColor(ccc3(255, 255, 0));
    hintConfirm->setScale(0.6);
    this->addChild(hintConfirm);

    auto labRemind = CCLabelBMFont::create("YES THIS TIME", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    labRemind->setScale(0.4);
    m_btnRemind = CCMenuItemSpriteExtra::create(labRemind, this, menu_selector(WarnCell::onClick));
    m_btnRemind->setPosition(CCPoint(210.f, 52.f));
    m_btnRemind->setTag(1);
    this->addChild(m_btnRemind);

    auto hintRemind = CCLabelBMFont::create(
        "Yes for now but please still remind me next time.",
        "ErasLight.fnt"_spr, 400.f, CCTextAlignment::kCCTextAlignmentCenter);
    hintRemind->setPosition(CCPoint(210.f, 40.f));
    hintRemind->setColor(ccc3(255, 255, 0));
    hintRemind->setScale(0.6);
    this->addChild(hintRemind);

    auto labEscape = CCLabelBMFont::create("EXIT", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    labEscape->setScale(0.4);
    this->m_btnEscape = CCMenuItemSpriteExtra::create(labEscape, this, menu_selector(WarnCell::onClick));
    this->m_btnEscape->setPosition(CCPoint(210.f, 20.f));
    this->m_btnEscape->setColor(ccc3(255, 96, 96));
    this->m_btnEscape->setTag(0);
    this->addChild(this->m_btnEscape);

    auto hintEscape = CCLabelBMFont::create(
        "Quit right now and tell the list team I not ever clicked the Chroma Icons button.",
        "ErasLight.fnt"_spr, 400.f, CCTextAlignment::kCCTextAlignmentCenter);
    hintEscape->setPosition(CCPoint(210.f, 8.f));
    hintEscape->setColor(ccc3(255, 255, 0));
    hintEscape->setScale(0.6);
    this->addChild(hintEscape);

    return true;
}

void WarnCell::onClick(CCObject* sender) {
    Signal<int>("warning").send(sender->getTag());
}

void WarnCell::Fade(bool in) {
    BaseCell::Fade(in);
    fade(this->m_text, in, ANIM_TIME_L, 1.2 * in, 1.2 * in);
}

bool TitleCell::init(const char* text, CCPoint pos, float width, int tag, std::string id) {
    if (!CCMenu::init())
        return false;

    // setup
    if (!BaseCell::setup(pos, CCSize(width, 20.f), tag, id))
        return false;

    auto mask = CCScale9Sprite::create("square.png"_spr, CCRect(0.f, 0.f, 80.f, 80.f));
    mask->setPosition(CCPoint(width/2 + 5.f, 15.f));
    mask->setContentSize(CCSize(width + 9.f, 29.f));

    this->m_clip = CCClippingNode::create();
    this->m_clip->setContentSize(CCSize(width + 10.f, 30.f));
    this->m_clip->setAnchorPoint(CCPoint(0.5, 0.5));
    this->m_clip->setStencil(mask);
    this->m_clip->setAlphaThreshold(0.3);
    this->m_clip->setZOrder(-1);

    this->m_spr = CCSprite::create("titlebg.png"_spr);
    this->m_spr->setTextureRect(CCRect(0, 0, width * 2, 90.f));
    this->m_spr->setOpacity(200);


    ccTexParams TexParameters = {GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT};
    this->m_spr->getTexture()->setTexParameters(&TexParameters);

    this->m_clip->addChild(this->m_spr);
    this->m_bg->addChildAtPosition(this->m_clip, Anchor::Center);

    // wtf
    this->m_spr->runAction(CCRepeatForever::create(CCSequence::create(
        CallFuncExt::create([this]() { m_spr->setPosition(CCPoint(275, 40.1)); }),
        CCMoveTo::create(30, CCPoint(40.5, -13)),
        nullptr
    )));

    this->m_title = CCLabelBMFont::create(text, "ErasBold.fnt"_spr, 120.f, CCTextAlignment::kCCTextAlignmentCenter);
    this->m_title->setScale(0.6);
    this->m_title->setContentSize(CCSize(width, 25.f));
    this->m_title->setWidth(300.f);
    this->m_title->setAnchorPoint(CCPoint(0.5, 0.5));
    this->m_title->setID("label");
    this->addChildAtPosition(this->m_title, Anchor::Center);

    this->m_pos = pos;
    return true;
}

void TitleCell::Fade(bool in) {
    this->BaseCell::Fade(in);
    fade(this->m_spr, in, ANIM_TIME_L, -1, -1, 144);
    fade(this->m_clip, in, ANIM_TIME_L, -1, -1, 144);
    this->runAction(CCSequence::create(
        CCEaseExponentialOut::create(CCMoveTo::create(
            ANIM_TIME_M, CCPoint(this->m_pos.x, this->m_pos.y - 30.f * in))),
        nullptr
    ));
}

// title option
bool OptionTitleCell::init(const char* text, float y, int tag, std::string id) {
    // parent
    if (!CCMenu::init())
        return false;

    this->m_title = CCLabelBMFont::create(text, "ErasBold.fnt"_spr, 120.f, CCTextAlignment::kCCTextAlignmentCenter);
    this->m_title->setScale(0.5);
    this->m_title->setContentSize(CCSize(300.f, 20.f));
    this->m_title->setWidth(340.f);
    this->m_title->setAnchorPoint(CCPoint(0.5, 0.5));
    this->m_title->setColor(ccc3(64, 192, 255));
    this->m_title->setID("title");
    this->addChild(this->m_title);

    // setup
    if (!BaseCell::setup(CCPoint(160.f, y-10.f), CCSize(300.f, 20.f), tag, id))
        return false;

    // set subnode position
    this->m_title->setPosition(CCPoint(150.f, 10.f));
    return true;
}

// toggler option
bool OptionTogglerCell::init(std::string title, float y, int tag, std::string id, std::string desc) {
    if (!CCMenu::init())
        return false;

    this->m_yes = Mod::get()->getSavedValue<bool>(id);
    this->m_title = title;
    this->m_desc = desc;

    this->m_toggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(OptionTogglerCell::onOption), 0.6);
    this->m_toggler->setPosition(ccp(290.f, 10.f));
    this->m_toggler->setCascadeOpacityEnabled(true);
    this->m_toggler->setID("toggler");
    this->m_toggler->toggle(this->m_yes);
    this->addChild(this->m_toggler);

    this->m_label = CCLabelBMFont::create(title.c_str(), "ErasBold.fnt"_spr, 240.f, CCTextAlignment::kCCTextAlignmentLeft);
    this->m_label->setPosition(ccp(5.f, 10.f));
    this->m_label->setScale(0.45);
    this->m_label->setContentSize(CCSize(275.f, 20.f));
    this->m_label->setAnchorPoint(CCPoint(0.f, 0.5f));
    this->m_label->setID("label");
    this->addChild(this->m_label);

    auto spr = CCSprite::create("infoBtn.png"_spr);
    spr->setScale(0.35);
    this->m_hint = CCMenuItemSpriteExtra::create(spr, this, menu_selector(OptionTogglerCell::onDesc));
    this->m_hint->setColor(ccc3(CELL_COLOR));
    
    this->addChild(this->m_hint);

    // setup
    if (!BaseCell::setup(CCPoint(160.f, y - 10.f), CCSize(300.f, 20.f), tag, id))
        return false;

    // switch!
    if (id == "activate") {
        this->m_label->setCString(this->m_yes ? "Switch : ON" : "Switch : OFF");
        this->m_label->setColor(ccc3(255 - 255 * this->m_yes, 255 * this->m_yes, 0));

        // tint bg
        this->tint(0, 80 * (!this->m_yes), 80 * (this->m_yes), 0);
    }
    this->m_hint->setPosition(ccp(this->m_label->getContentWidth() * 0.45 + 15.f, 10.f));    

    return true;
}

void OptionTogglerCell::onOption(CCObject* sender) {
    // revert
    this->m_yes = !this->m_yes;
    // set value
    Mod::get()->setSavedValue(this->getID(), this->m_yes);
    // option
    opts[this->getID()] = this->m_yes;  
    // post signal
    Signal<bool>(this->getID()).send(this->m_yes);

    // switch
    if (this->getID() == "activate") {
        this->m_label->setCString(this->m_yes ? "Switch : ON" : "Switch : OFF");
        this->m_label->setColor(ccc3(255 - 255 * this->m_yes, 255 * this->m_yes, 0));
        // tint bg
        this->tint(ANIM_TIME_M, 80 * !this->m_yes, 80 * this->m_yes, 0);
        // desc button
        this->m_hint->setPosition(ccp(this->m_label->getContentWidth() * 0.45 + 20.f, 10.f));
    }
}

void OptionTogglerCell::onDesc(CCObject* sender) {
    log::debug("title = {} desc = {}", this->m_title, this->m_desc);
    Signal<std::pair<std::string, std::string>>("option-desc").send({this->m_title, this->m_desc});
}

void OptionTogglerCell::switchTheme() {
    BaseCell::switchTheme();
    this->m_hint->setColor(ccc3(CELL_COLOR));
}

bool OptionArrowCell::init(std::string title, float y, int tag, std::string id, std::string desc, std::vector<int> enums, std::function<std::string (int)> getReal) {
    if (!CCMenu::init())
        return false;

    this->m_index = Mod::get()->getSavedValue<int64_t>(id);
    int len = this->m_enums.size();
    // clamp
    this->m_index = this->m_index < 0 ? 0 : (this->m_index > len ? len : this->m_index);

    this->m_title = title;
    this->m_desc = desc;
    this->m_enums = enums;
    this->getReal = getReal;

    this->m_label = CCLabelBMFont::create(title.c_str(), "ErasBold.fnt"_spr, 240.f, CCTextAlignment::kCCTextAlignmentLeft);
    this->m_label->setPosition(ccp(5.f, 10.f));
    this->m_label->setScale(0.45);
    this->m_label->setContentSize(CCSize(275.f, 20.f));
    //m_label->setWidth(340.f);
    this->m_label->setAnchorPoint(CCPoint(0.f, 0.5f));
    this->m_label->setID("label");
    addChild(this->m_label);

    this->m_display = CCLabelBMFont::create(
        (getReal(this->m_index)).c_str(),
        "ErasBold.fnt"_spr, 240.f, CCTextAlignment::kCCTextAlignmentCenter);
    this->m_display->setPosition(ccp(260.f, 10.f));
    this->m_display->setScale(0.45);
    this->m_display->setContentSize(CCSize(180.f, 20.f));
    this->m_display->setID("display");
    // this line is NECESSARY, don't ask why i just know it's needed
    this->m_display->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
    this->addChild(this->m_display);

    auto spr = CCSprite::create("infoBtn.png"_spr);
    spr->setScale(0.35);
    this->m_hint = CCMenuItemSpriteExtra::create(spr, this, menu_selector(OptionArrowCell::onDesc));
    this->m_hint->setColor(ccc3(CELL_COLOR));
    
    this->addChild(this->m_hint);

    // arrow left
    auto sprLeft = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    sprLeft->setScale(0.3f);
    sprLeft->setFlipX(true);
    this->m_btnArrowL = CCMenuItemSpriteExtra::create(sprLeft, this, menu_selector(OptionArrowCell::onArrow));
    this->m_btnArrowL->setPosition(CCPoint(230.f, 10.f));
    this->m_btnArrowL->setTag(-1);
    this->addChild(this->m_btnArrowL);

    // arrow right
    auto sprRight = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    sprRight->setScale(0.3f);
    this->m_btnArrowR = CCMenuItemSpriteExtra::create(sprRight, this, menu_selector(OptionArrowCell::onArrow));
    this->m_btnArrowR->setPosition(CCPoint(290.f, 10.f));
    this->m_btnArrowR->setTag(1);
    this->addChild(this->m_btnArrowR);


    // setup
    if (!BaseCell::setup(CCPoint(160.f, y - 10.f), CCSize(300.f, 20.f), tag, id))
        return false;

    this->m_hint->setPosition(ccp(m_label->getContentWidth() * 0.45 + 15.f, 10.f));

    return true;
}

void OptionArrowCell::onArrow(CCObject* sender) {
    int len = this->m_enums.size();
    this->m_index = (this->m_index + sender->getTag() + len) % len;
    this->m_display->setString(this->getReal(this->m_index).c_str());
    vals[this->getID()] = this->m_index;
    Mod::get()->setSavedValue(this->getID(), this->m_index);
    Signal<int>(this->getID()).send(this->m_index);
}

void OptionArrowCell::onDesc(CCObject* sender) {
    log::debug("title = {} desc = {}", m_title, m_desc);
    Signal<std::pair<std::string, std::string>>("option-desc").send({this->m_title, this->m_desc});
}

void OptionArrowCell::switchTheme() {
    this->BaseCell::switchTheme();
    this->m_hint->setColor(ccc3(CELL_COLOR));
}

bool OptionSliderCell::init(const char* title, float y, int tag, std::string id, std::string desc, float min, float max, int precision,
    std::function<float (float)> toSlider, std::function<float (float)> fromSlider) {
    if (!CCMenu::init())
        return false;

    this->setTag(tag);

    this->m_value = Mod::get()->getSavedValue<float>(id);
    this->m_min = min;
    this->m_max = max;
    this->m_precision = precision;

    this->m_toSlider = toSlider;
    this->m_fromSlider = fromSlider;

    this->m_title = title;
    this->m_desc = desc;

    this->m_label = CCLabelBMFont::create(title, "ErasBold.fnt"_spr, 240.f, CCTextAlignment::kCCTextAlignmentLeft);
    this->m_label->setPosition(ccp(5.f, 10.f));
    this->m_label->setScale(0.45);
    this->m_label->setContentSize(CCSize(275.f, 20.f));
    this->m_label->setAnchorPoint(CCPoint(0.f, 0.5f));
    this->m_label->setID("label");
    this->addChild(this->m_label);

    auto spr = CCSprite::create("infoBtn.png"_spr);
    spr->setScale(0.35f);
    this->m_hint = CCMenuItemSpriteExtra::create(spr, this, menu_selector(OptionSliderCell::onDesc));
    this->m_hint->setColor(ccc3(CELL_COLOR));
    this->addChild(this->m_hint);

    this->m_slider = Slider::create(this, menu_selector(OptionSliderCell::onSlider));
    this->m_slider->setPosition(ccp(250.f, 10.f));
    this->m_slider->setContentSize(ccp(0.f, 0.f));
    this->m_slider->setScale(0.45f);
    this->m_slider->setID("slider");
    this->m_slider->setValue(this->m_toSlider(this->m_value));
    this->m_slider->m_delegate = this;
    this->addChild(this->m_slider);

    this->m_display = CCLabelBMFont::create(
        (precision ? numToString(this->m_value, precision) : numToString<int>(this->m_value, precision)).c_str(),
        "ErasBold.fnt"_spr, 240.f, CCTextAlignment::kCCTextAlignmentLeft);
    this->m_display->setPosition(ccp(200.f, 10.f));
    this->m_display->setScale(0.45);
    this->m_display->setContentSize(CCSize(50.f, 20.f));
    this->m_display->setAnchorPoint(CCPoint(1.f, 0.5f));
    this->m_display->setID("display");
    this->addChild(this->m_display);

    // setup
    if (!BaseCell::setup(CCPoint(160.f, y - 10.f), CCSize(300.f, 20.f), tag, id))
        return false;

    this->m_hint->setPosition(ccp(this->m_label->getContentWidth() * 0.45 + 15.f, 10.f));
    return true;
}

void OptionSliderCell::sliderBegan(Slider* slider) {
    Signal<bool>("drag-slider").send(true);
}

void OptionSliderCell::sliderEnded(Slider* slider) {
    Signal<bool>("drag-slider").send(false);
    vals[this->getID()] = this->m_value;
    Mod::get()->setSavedValue(this->getID(), this->m_value);
    this->postEvent();
}

void OptionSliderCell::onDesc(CCObject* sender) {
    log::debug("title = {} desc = {}", this->m_title, this->m_desc);
    Signal<std::pair<std::string, std::string>>("option-desc").send({this->m_title, this->m_desc});
}

void OptionSliderCell::onSlider(CCObject* sender) {
    this->m_value = this->m_fromSlider(m_slider->getValue());
    if (this->m_precision)
        this->m_display->setCString(numToString(this->m_value, this->m_precision).c_str());
    else
        this->m_display->setCString(numToString((int)this->m_value).c_str());
}

void OptionSliderCell::postEvent() {
    if (this->m_precision)
        Signal<float>(this->getID()).send(this->m_value);
    else
        Signal<int>(this->getID()).send((int)this->m_value);
}


bool ItemCell::init(int tag) {
    if (!CCMenu::init())
        return false;

    // effect
    if (tag == 3) {
        // setup
        if (!BaseCell::setup(CCPoint(0.f, -65.f), CCSize(180.f, 24.f), tag, "effect-sheet"))
            return false;

        for (int item = 10; item < 16; item ++) {
            // spr
            auto btn = PickItemButton::create(item, false, this, menu_selector(ItemCell::onPickItem));
            btn->setPosition(CCPoint(30.f*item - 285.f, 12.f));
            btn->setScale(0);
            btn->setOpacity(0);
            this->addChild(btn);
            this->m_btns.push_back(btn);
        }        
    }
    // easy mode icons
    else if (tag == 2) {
        // setup
        if (!BaseCell::setup(CCPoint(0.f, 10.f), CCSize(50.f, 50.f), tag, "easy-sheet"))
            return false;
        // btn
        auto btn = PickItemButton::create(0, false, this, menu_selector(ItemCell::onPickItem));
        btn->setPosition(CCPoint(25.f, 25.f));
        // init status
        btn->setScale(0);
        btn->setOpacity(0);
        this->addChild(btn);
        this->m_btns.push_back(btn);
    }
    // advanced icons
    else {
        // setup
        if (!BaseCell::setup(CCPoint(0.f, 10.f), CCSize(400.f, 34.f), tag, "advanced-sheet"))
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
            this->m_btns.push_back(btn);
        }
    }
    
    // say goodbye to init
    this->m_bg->setTag(100);
    this->m_bg->setScale(5, 0.2);
    this->m_bg->setOpacity(0);

    return true;
}

void ItemCell::onPickItem(CCObject* sender) {
    Signal<int>("pick").send(sender->getTag());
}

void ItemCell::Fade(bool in) {
    // bg
    fade(this->m_bg, in, ANIM_TIME_M, in ? 1 : 5, in ? 1 : 0.2);
    // items
    int length = this->m_btns.size();
    for (int i = 0; i < length; i++) {
        this->m_btns[i]->delayFade(in ? abs((length - 1 - 2 * i) / 2) : 0, in);
    }
}

void ItemCell::runChroma(float const& phase, float const& percentage, int const& progress) {
    for (auto btn : this->m_btns)
        btn->runChroma(phase, percentage, progress);
}

void ItemCell::toggleChroma() {
    for (auto btn : this->m_btns)
        btn->toggleChroma();
}

void ItemCell::switchPlayer() {
    for (auto btn : this->m_btns)
        btn->switchPlayer();
}

void ItemCell::setModeTarget(Gamemode gamemode) {
    // refresh item menu target
    for (auto btn : this->m_btns)
        btn->setModeTarget(gamemode);
}

bool SetupItemCell::init(int index, float Y, int tag) {
    if (!CCMenu::init())
        return false;

    if (!BaseCell::setup(CCPoint(55.f, Y), CCSize(90.f, 24.f), tag, "sheet"))
        return false;

    this->m_index = index;

    this->m_label = CCLabelBMFont::create((index > 9 ? chnls[index - 6] : items[index]).c_str(), "ErasBold.fnt"_spr, 120.f, CCTextAlignment::kCCTextAlignmentLeft);
    this->m_label->setScale(0.4);
    // wave trail and ghost trail labels are too long
    if (index == 10 || index == 14)
        this->m_label->setScaleX(0.35);
    this->m_label->setPosition(CCPoint(60.f, 12.f));
    this->m_label->setContentSize(CCSize(60.f, 24.f));
    this->m_label->setWidth(65.f);
    this->m_label->setColor(ccc3(127, 127, 127));
    this->m_label->setID("label");
    this->addChild(this->m_label);

    // btn
    this->m_btn = PickItemButton::create(index, true, this, menu_selector(SetupItemCell::onPickItem));
    this->m_btn->setPosition(CCPoint(14.f, 12.f));
    this->addChild(this->m_btn);

    this->setCascadeOpacityEnabled(true);
    return true;
}

void SetupItemCell::onPickItem(CCObject* sender) {
    Signal<int>("pick").send(this->m_index);
}

void SetupItemCell::switchPlayer() {
    this->m_btn->switchPlayer();
}

void SetupItemCell::toggleChroma(bool on) {
    this->m_btn->toggleChroma(on);
}

void SetupItemCell::runChroma(float phase, float percentage, int progress) {
    this->m_btn->runChroma(phase, percentage, progress);
}

void SetupItemCell::select(bool current) {
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

bool SetupItemCell::setModeTarget(Gamemode gamemode) {
    if (this->m_index > 9)
        this->m_btn->setModeTarget(gamemode);
    return this->m_index > 9;
}

bool SetupOptionCell::init() {
    if (!BaseCell::setup(CCPoint(55.f, 0.f), CCSize(240.f, 230.f), 0, "setup-options"))
        return false;

    this->setVisible(false);
    this->setOpacity(0.f);
    this->setScale(0.5);
    this->setCascadeOpacityEnabled(true);
    // node ids
    std::vector<std::vector<int>> array = {{1}, {1, 3}, {1, 6, 5, 5, 5}, {1, 4, 5}, {1, 4, 6}};
    int t = 0;
    for (int mode = 0; mode < 5; mode++) {
        for (auto type: array.at(mode)) {
            // tag plus one
            t ++;
            // construct
            auto optionNode = SetupOptionLine::create(OptionLineType(type), mode, t);
            this->addChild(optionNode);
            this->m_aryMenus.push_back(optionNode);
        }
    }
    return true;
}

void SetupOptionCell::refreshUI(ChromaPattern setup, bool fade) {
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
    for (auto node: this->m_aryMenus) {
        // title toggle and tint
        if (node->m_type == OptionLineType::Title) {
            Y -= 5.f;            
            node->toggleTitle(node->m_mode == setup.mode, fade);
        } else
            switch (node->m_type) {
            case OptionLineType::Color:
                node->m_colpk->setColor(setup.color);
                break;
            case OptionLineType::MultiColor:
                node->m_colpk1->setColor(node->m_mode == 4 ? setup.progress.begin()->second : grad1);
                node->m_colpk2->setColor(node->m_mode == 4 ? (setup.progress.rbegin())->second : grad2);
                break;
            case OptionLineType::Toggler:
                node->m_toggler->toggle(node->getTag() == 14 ? setup.best : setup.rev);
                break;
            case OptionLineType::Slider:
                switch (node->getTag()) {
                case 6:
                    node->setVal(setup.phase);
                    break;
                case 7:
                    node->setVal(setup.satu);
                    break;
                case 8:
                    node->setVal(setup.brit);
                    break;
                default:
                    node->setVal((int)(duty / 3.6));
                    break;
                }
                break;
            default:
                break;
            }

        bool display = node->m_type == OptionLineType::Title || node->m_mode == setup.mode;
        auto action = CCSpawn::create(
            CCEaseExponentialOut::create(CCMoveTo::create(fade*ANIM_TIME_M, CCPoint(this->getContentWidth() / 2, Y))),
            CCEaseExponentialOut::create(CCScaleTo::create(fade*ANIM_TIME_M, 1, display)),
            CCEaseExponentialOut::create(CCFadeTo::create(fade*ANIM_TIME_M, display*255)),
            CallFuncExt::create([node, display](void){ if (node->m_type == OptionLineType::Slider) node->helpFade(display); }),
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
    static_cast<SliderBundleBase*>(getChildByID("phase"))->helpFade(in);
    static_cast<SliderBundleBase*>(getChildByID("satu"))->helpFade(in);
    static_cast<SliderBundleBase*>(getChildByID("brit"))->helpFade(in);
    static_cast<SliderBundleBase*>(getChildByID("duty"))->helpFade(in);
    // move
    if (dir)
        this->runAction(CCEaseExponentialOut::create(
            CCMoveTo::create(ANIM_TIME_M, CCPoint(50.f - 200 * dir * (1-in), 0.f))));
}

std::string rgbLabels[3] = {"R", "G", "B"};

bool ColorValueCell::init(int type) {
    if (!CCMenu::init())
        return false;

    this->m_type = type;
    // label
    this->m_label = CCLabelBMFont::create(rgbLabels[type].c_str(), "ErasBold.fnt"_spr, 15.f, CCTextAlignment::kCCTextAlignmentLeft);
    this->m_label->setPosition(CCPoint(10.f, 10.f));
    this->m_label->setAnchorPoint(CCPoint(0.f, 0.5));
    this->m_label->setColor(ccc3(200 + 55 * (type==0), 200 + 55 * (type==1), 200 + 55 * (type==2)));
    this->m_label->setScale(0.5);
    this->addChild(this->m_label);

    // inputer
    this->m_inputer = TextInput::create(50.f, rgbLabels[type].c_str(), "ErasBold.fnt"_spr);
    this->m_inputer->setPosition(CCPoint(140.f, 10.f));
    this->m_inputer->setFilter("1234567890");
    this->m_inputer->setMaxCharCount(3);
    this->m_inputer->setDelegate(this);
    this->m_inputer->getChildByType<NineSlice>(0)->setVisible(false);
    this->m_inputer->setScale(0.9);
    this->m_inputer->setID("text-input");
    this->addChild(this->m_inputer);

    // slider
    this->m_slider = Slider::create(this, menu_selector(ColorValueCell::onSlider), 0.4);
    this->m_slider->setPosition(CCPoint(70.f, 10.f));
    this->m_slider->m_delegate = this;
    this->m_slider->setID("slider");
    this->addChild(this->m_slider);

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
        this->m_value = numFromString<int>(input).unwrapOr(255);
        this->m_value = this->m_value > 255 ? 255 : this->m_value;
        m_slider->setValue(this->Val2Slider(this->m_value));
        Signal<int>("color-" + rgbLabels[this->m_type]).send(this->m_value);
    }
}

void ColorValueCell::textInputClosed(CCTextInputNode* p) {
    std::string input = p->getString();
    this->m_value = numFromString<float>(input).unwrapOr(this->m_value);
    p->setString(input);
    Signal<int>("color-" + rgbLabels[this->m_type]).send(this->m_value);
}

void ColorValueCell::onSlider(CCObject* sender) {
    this->m_value = this->Slider2Val(this->m_slider->getValue());
    this->m_inputer->setString(numToString(this->m_value));
    Signal<int>("color-" + rgbLabels[this->m_type]).send(this->m_value);
}
void ColorValueCell::sliderBegan(Slider *p) {
    Signal<bool>("drag-slider").send(true);
    // tint bg
    this->tint(ANIM_TIME_M, 50 * (this->m_type==0), 50 * (this->m_type==1), 50 * (this->m_type==2));
}
void ColorValueCell::sliderEnded(Slider *p) {
    Signal<bool>("drag-slider").send(false);
    // tint bg
    this->m_bg->runAction(CCTintTo::create(ANIM_TIME_M, CELL_COLOR));
    Signal<int>("color-" + rgbLabels[this->m_type]).send(this->m_value);
};

// value -> slider
float ColorValueCell::Val2Slider(int val) {
    return std::clamp((float)val, 0.f, 255.f) / 255;
}
// slider -> value
int ColorValueCell::Slider2Val(float s) {
    return round(s * 255);
}

int ColorValueCell::getVal() {
    return this->m_value;
}
void ColorValueCell::setVal(int value) {
    this->m_value = value;
    this->m_inputer->setString(numToString(value));
    this->m_slider->setValue(Val2Slider(value));
}

void ColorValueCell::Fade(bool in) {
    BaseCell::Fade(in);
    fadeSlider(this->m_slider, in);

    this->m_inputer->getChildByType<CCTextInputNode>(0)->getChildByType<CCLabelBMFont>(0)->runAction(
        CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255 * in)));     
}

bool ColorHexCell::init() {
    if (!CCMenu::init())
        return false;

    // label
    this->m_label = CCLabelBMFont::create("HEX", "ErasBold.fnt"_spr, 40.f, CCTextAlignment::kCCTextAlignmentLeft);
    this->m_label->setPosition(CCPoint(10.f, 10.f));
    this->m_label->setAnchorPoint(CCPoint(0.f, 0.5));
    this->m_label->setColor(ccc3(200, 200, 200));
    this->m_label->setScale(0.5);
    this->addChild(this->m_label);

    // inputer
    this->m_inputer = TextInput::create(90.f, "HEX", "ErasBold.fnt"_spr);
    this->m_inputer->setPosition(CCPoint(95.f, 10.f));
    this->m_inputer->setFilter("1234567890ABCDEFabcdef");
    this->m_inputer->setMaxCharCount(6);
    this->m_inputer->setDelegate(this);
    this->m_inputer->getChildByType<NineSlice>(0)->setVisible(false);
    this->m_inputer->setID("text-input");
    this->m_inputer->setScale(0.9);
    this->addChild(this->m_inputer);

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
    this->m_str = p->getString();
    if (auto color = cc3bFromHexString(p->getString(), true))
        Signal<ccColor3B>("color-hex").send(color.unwrap());
}

void ColorHexCell::Fade(bool in) {
    BaseCell::Fade(in);
    this->m_inputer->getChildByType<CCTextInputNode>(0)->getChildByType<CCLabelBMFont>(0)->runAction(
        CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255 * in)));     
}

void ColorHexCell::setColorValue(ccColor3B const& color) {
    this->m_str = cc3bToHexString(color);
    this->m_inputer->setString(this->m_str);
}