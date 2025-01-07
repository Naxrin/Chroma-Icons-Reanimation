#include "mynode.hpp"

extern std::map<short, ChromaSetup> setups;
extern std::map<std::string, bool> opts;

// UnlockType tags by RobTop
const static int gametype[9] = {1, 4, 5, 6, 7, 8, 9, 13, 14};
// IconType name for More Icons
const static std::string MInames[9] = {"icon", "ship", "ball", "ufo", "wave", "robot", "spider", "swing", "jetpack"};
// IconType and Colors name for Separate Dual Icons
const static std::string SDInames[9] = {"cube", "ship", "roll", "bird", "dart", "robot", "spider", "swing", "jetpack"};
//player mode
const static int garageIconIndex[9] = {
    gm->getPlayerFrame(), gm->getPlayerShip(), gm->getPlayerBall(),
    gm->getPlayerBird(), gm->getPlayerDart(), gm->getPlayerRobot(),
    gm->getPlayerSpider(), gm->getPlayerSwing(), gm->getPlayerJetpack(),
};

bool PickItemButton::init(int tag, bool src, CCObject* target, cocos2d::SEL_MenuHandler callback) {
    this->setTag(tag);
    this->src = src;

    // effect
    if (tag > 10) {
        // spr
        effect = GJItemEffect::createEffectItem(tag - 10);
        effect->setScale(src ? 0.75 : 0.85);
        // init frame color
        this->setPlayerStatus();
        // init button
        return CCMenuItemSpriteExtra::init(effect, effect, target, callback);
    }
    // init color

    // icon
    // spr
    int index = tag ? tag - 1 : 0;
    icon = GJItemIcon::createBrowserItem(UnlockType(gametype[index]), garageIconIndex[index]);
    if (gm->m_playerGlow)
        icon->m_player->setGlowOutline(ccc3(255, 255, 255));
    if (src)
        icon->setScale(0.6);
    else if (tag)
        icon->setScale(0.8);

    // init frame and color
    this->setPlayerStatus();

    // cascade opacity ohh fuck
    icon->setCascadeOpacityEnabled(true);
    icon->m_player->setCascadeOpacityEnabled(true);
    for (CCObject* obj: CCArrayExt<CCObject*>(icon->m_player->getChildren()))
        static_cast<CCSprite*>(obj)->setCascadeOpacityEnabled(true);
    if (index == 5 || index == 6) {
        auto part = icon->m_player->getChildByType<CCAnimatedSprite>(0)->getChildByType<CCPartAnimSprite>(0);
        part->setCascadeOpacityEnabled(true);
        for (CCObject* obj: CCArrayExt<CCObject*>(part->getChildren()))
            static_cast<CCSprite*>(obj)->setCascadeOpacityEnabled(true);
    }
    // init button
    return CCMenuItemSpriteExtra::init(icon, icon, target, callback);
}

void PickItemButton::setPlayerStatus() {
    int index = this->getTag() ? this->getTag() - 1 : 0;
    int f, m, s, g;
    
    // Sep Dual Icons compatible
    auto SDI = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
    // frame
    if (index < 10) {
        // update frames 
        f = (SDI && ptwo) ? SDI->getSavedValue<int64_t>(SDInames[index]) : garageIconIndex[index];
        this->icon->m_player->updatePlayerFrame(f, IconType(index));
        // More Icons compatible
        if (auto MI = Loader::get()->getLoadedMod("hiimjustin000.more_icons")) {
            // config
            auto name = MI->getSavedValue<std::string>(MInames[index] + (ptwo ? "-dual" : ""));
            // post event
            DispatchEvent<SimplePlayer*, std::string, IconType>(
                "hiimjustin000.more_icons/simple-player", icon->m_player, name, IconType(index)).post();
        }           
    }
    if (SDI && ptwo) {
        // colors
        m = SDI->getSavedValue<int64_t>("color1");
        s = SDI->getSavedValue<int64_t>("color2");
        g = SDI->getSavedValue<int64_t>("colorglow");
    } else {
        m = ptwo ? gm->getPlayerColor2() : gm->getPlayerColor();
        s = ptwo ? gm->getPlayerColor() : gm->getPlayerColor2();
        g = gm->getPlayerGlowColor();
    }

    this->mainColor = gm->colorForIdx(m);
    this->secondColor = gm->colorForIdx(s);
    this->glowColor = gm->colorForIdx(g);
}

void PickItemButton::delayFade(int delay, bool in) {
    this->runAction(CCSequence::create(
        CCDelayTime::create(0.01 + ANIM_TIME_GAP*(1+delay)),
        CallFuncExt::create([this, in](void) {
            fade(this, in, ANIM_TIME_L);
        }),
        nullptr
    ));
}

void PickItemButton::runChroma(float const& phase, float const& percentage, float const& progress) {
    if (!this->chroma)
        return;
    if (this->getTag() > 10)
        this->effect->setColor(getChroma(
            setups[getIndex(ptwo, getTag(), (this->getTag() == 12 || this->getTag() == 15) ? 4 : effect->targetMode)],
            this->mainColor, phase, percentage, progress));
    else {
        // main
        icon->m_player->setColor(getChroma(
            setups[getIndex(ptwo, getTag(), 0)], this->mainColor, phase, percentage, progress));
        // second
        icon->m_player->setSecondColor(getChroma(
            setups[getIndex(ptwo, getTag(), 1)], this->secondColor, phase + 120 * opts["sep-second"], percentage, progress));
        // glow
        if (icon->m_player->m_hasGlowOutline)
            icon->m_player->setGlowOutline(getChroma(
                setups[getIndex(ptwo, getTag(), 2)], this->glowColor, phase + 240 * opts["sep-glow"], percentage, progress));

        // white
        auto white = getChroma(setups[getIndex(ptwo, getTag(), 3)], ccc3(255, 255, 255), phase, percentage, progress);
        if (icon->m_unlockType == UnlockType::Robot)
            icon->m_player->m_robotSprite->getChildByType<CCPartAnimSprite>(0)->getChildByTag(1)->getChildByType<CCSprite>(1)->setColor(white);
        else if (icon->m_unlockType == UnlockType::Spider)
            icon->m_player->m_spiderSprite->getChildByType<CCPartAnimSprite>(0)->getChildByTag(1)->getChildByType<CCSprite>(1)->setColor(white);
        else
            icon->m_player->m_detailSprite->setColor(white);
    }
}

void PickItemButton::toggleChroma() {
    this->toggleChroma(opts["activate"] && opts["prev"]);
}

// toggle on or off chroma, activated means not grey
void PickItemButton::toggleChroma(bool current) {
    // toggle
    this->chroma = current;
    // on
    if (this->chroma)
        return;
    // off
    if (this->getTag() > 9)
        this->effect->setColor(opts["activate"] ? this->mainColor : ccc3(127, 127, 127));
    else {
        auto p = this->icon->m_player;
        p->setColor(opts["activate"] ? this->mainColor : ccc3(175, 175, 175));
        p->setSecondColor(opts["activate"] ? this->secondColor : ccc3(255, 255, 255));
        if (p->m_hasGlowOutline)
            p->setGlowOutline(opts["activate"] ? this->glowColor : ccc3(255, 255, 255));

        // white
        if (icon->m_unlockType == UnlockType::Robot)
            icon->m_player->m_robotSprite->getChildByType<CCPartAnimSprite>(0)->getChildByTag(1)->getChildByType<CCSprite>(1)
                ->setColor(ccc3(255, 255, 255));
        else if (icon->m_unlockType == UnlockType::Spider)
            icon->m_player->m_spiderSprite->getChildByType<CCPartAnimSprite>(0)->getChildByTag(1)->getChildByType<CCSprite>(1)
                ->setColor(ccc3(255, 255, 255));
        else
            icon->m_player->m_detailSprite->setColor(ccc3(255, 255, 255));
    }
}

bool SliderBundleBase::init(const char* name, float value, float max) {
    if (!CCMenu::init())
        return false;

    this->max = max;
    this->ignoreAnchorPointForPosition(false);
    this->setVisible(false);

    // label
    m_label = CCLabelBMFont::create(name, "ErasBold.fnt"_spr, 140.f, CCTextAlignment::kCCTextAlignmentLeft);
    m_label->setPosition(CCPoint(0.f, 10.f));
    m_label->setAnchorPoint(CCPoint(0.f, 0.5));
    this->addChild(m_label);    

    // inputer
    this->m_inputer = TextInput::create(60.f, name, "ErasBold.fnt"_spr);
    m_inputer->setFilter("1234567890.");
    m_inputer->setDelegate(this);
    m_inputer->getChildByType<CCScale9Sprite>(0)->setVisible(false);
    m_inputer->setID("text-input");
    this->addChild(m_inputer);

    // arrow left
    auto arrowLeft = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    arrowLeft->setScale(0.3);
    arrowLeft->setFlipX(true);
    m_btnLeft = CCMenuItemSpriteExtra::create(arrowLeft, this, menu_selector(SliderBundleBase::onArrow));
    m_btnLeft->setTag(1);    
    this->addChild(m_btnLeft);

    // arrow right
    auto arrowRight = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
    arrowRight->setScale(0.3);
    m_btnRight = CCMenuItemSpriteExtra::create(arrowRight, this, menu_selector(SliderBundleBase::onArrow));
    m_btnRight->setTag(2);
    this->addChild(m_btnRight);

    // slider
    this->m_slider = Slider::create(this, menu_selector(SliderBundleBase::onSlider), 0.6);
    m_slider->m_delegate = this;
    this->addChild(m_slider);

    this->setVal(value);
    return true;
}

void SliderBundleBase::textChanged(CCTextInputNode* p) {
    std::string input = p->getString();
    if (input != "")
        m_slider->setValue(Val2Slider(stof(input)));
}

void SliderBundleBase::textInputClosed(CCTextInputNode* p) {
    std::string input = p->getString();
    if (input == "")
        input = cocos2d::CCString::createWithFormat("%.2f", static_cast<float>(value))->getCString();
    else {
        value = stof(input);
        if (max > 0 && value > max)
            value = max;
        input = cocos2d::CCString::createWithFormat("%.2f", static_cast<float>(value))->getCString();
    }
    p->setString(input);
    postEvent();
}

void SliderBundleBase::onSlider(CCObject* sender) {
    value = round(Slider2Val(m_slider->getValue()) * 100) / 100;
    m_inputer->setString(cocos2d::CCString::createWithFormat("%.2f", static_cast<float>(value))->getCString());
}


void SliderBundleBase::onArrow(CCObject* sender) {
    float delta = sender->getTag() == 2 ? 0.1 : -0.1;
    float s = m_slider->getValue();
    s += delta;
    s = s > 1 ? 1 : (s < 0 ? 0 : s);
    
    value = Slider2Val(s);
    if (max > 0 && value > max) {
        value = max;
        m_slider->setValue(Val2Slider(max));
    } else
        m_slider->setValue(s);

    m_inputer->setString(cocos2d::CCString::createWithFormat("%.2f", static_cast<float>(value))->getCString());
    postEvent();
}

bool SpeedSliderBundle::init(CCPoint pos, const char* name, float val, float max, int tag, std::string id) {
    if (!SliderBundleBase::init(name, val, max))
        return false;

    this->setID(id);
    this->setPosition(pos);
    this->setAnchorPoint(CCPoint(0.5, 0.5));
    this->setContentSize(CCSize(320.f, 20.f));
    this->setScale(5, 0.2);

    // label
    m_label->setScale(0.6);
    // inputer
    m_inputer->setPosition(CCPoint(275.f, 10.f));
    m_inputer->setScale(0.8);
    // slider
    m_slider->setPosition(CCPoint(160.f, 10.f));
    // arrows
    m_btnLeft->setPosition(CCPoint(240.f, 10.f));
    m_btnRight->setPosition(CCPoint(310.f, 10.f));

    return true;
}

void SpeedSliderBundle::Fade(bool in) {
    fade(this, in, ANIM_TIME_M, in ? 1 : 5, in ? 1 : 0.2);
    // manually cascade opacity
    m_slider->getChildByType<CCSprite>(0)->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255*in)));
    m_slider->m_sliderBar->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255*in)));
    m_slider->getThumb()->getChildByTag(1)->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255*in)));
    m_inputer->getChildByType<CCTextInputNode>(0)->getChildByType<CCLabelBMFont>(0)->runAction(
        CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255*in)));
}

const char* titles[5] = {"Default", "Static", "Chromatic", "Gradient", "Progress"};
const char* descs[5] = {
    "As if this mod isn't loaded here.",
    "Set to your given static one.",
    "Just Chroma like the name of this mod. Set saturation percent to 50 if you want pastel like icons",
    "Pick two colors to cycle gradient or set duty value close to 100 or 0 so it looks like pulse.",
    "Let this color gradient from one to another during your gameplay. In plat levels your current progress is always regarded 0."
};

bool SetupOptionLine::init(OptionLineType type, int mode, int tag) {
    if (!CCMenu::init())
        return false;

    this->type = type;
    this->mode = mode;
    // common constructor
    this->setTag(tag);
    this->setContentSize(CCSize(220.f, 20.f));
    this->setAnchorPoint(CCPoint(0.5, 0.5));
    this->ignoreAnchorPointForPosition(false);
    
    ColorChannelSprite* sqr;
    CCLabelBMFont* label;
    // customize
    switch (type) {
    case OptionLineType::Title:
        this->setContentHeight(25.f);    

        this->m_title = CCLabelBMFont::create(titles[mode], "ErasBold.fnt"_spr, 240.f, CCTextAlignment::kCCTextAlignmentLeft);
        m_title->setScale(0.5);
        m_title->setPosition(CCPoint(25.f, 10.f));
        m_title->setAnchorPoint(CCPoint(0.f, 0.5));
        this->addChild(m_title);

        m_toggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(SetupOptionLine::onToggle), 0.55);
        m_toggler->setPosition(CCPoint(10.f, 10.f));
        m_toggler->setCascadeOpacityEnabled(true);
        this->addChild(m_toggler);
        break;
    case OptionLineType::Toggler:
        label = CCLabelBMFont::create("Best Progress", "ErasBold.fnt"_spr, 220.f, CCTextAlignment::kCCTextAlignmentLeft);
        label->setScale(0.4);
        label->setPosition(CCPoint(25.f, 10.f));
        label->setAnchorPoint(CCPoint(0.f, 0.5));
        this->addChild(label);

        m_toggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(SetupOptionLine::onToggle), 0.5);
        m_toggler->setPosition(CCPoint(10.f, 10.f));
        m_toggler->setCascadeOpacityEnabled(true);        
        this->addChild(m_toggler);
        break;
    case OptionLineType::Desc:
        label = CCLabelBMFont::create(descs[mode], "ErasLight.fnt"_spr, 220.f, CCTextAlignment::kCCTextAlignmentLeft);
        label->setScale(0.8);
        label->setPosition(CCPoint(10.f, 10.f));
        label->setAnchorPoint(CCPoint(0.f, 0.5));
        label->setColor(ccc3(255, 255, 0));
        this->addChild(label);

        this->setContentHeight(label->getContentHeight());
        break;
    case OptionLineType::SingleColor:
        sqr = ColorChannelSprite::create();
        sqr->setScale(0.5);
        this->m_colpk = CCMenuItemSpriteExtra::create(sqr, this, menu_selector(SetupOptionLine::onPickColor));
        m_colpk->setPosition(CCPoint(10.f, 10.f));
        m_colpk->setID("static");
        m_colpk->setTag(1);
        this->addChild(m_colpk);

        label = CCLabelBMFont::create("Pick your color here", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentLeft);
        label->setScale(0.4);
        label->setPosition(CCPoint(25.f, 10.f));
        label->setAnchorPoint(CCPoint(0.f, 0.5));
        this->addChild(label);
        break;
    case OptionLineType::MultiColor:
        sqr = ColorChannelSprite::create();
        sqr->setScale(0.5);
        this->m_colpk1 = CCMenuItemSpriteExtra::create(sqr, this, menu_selector(SetupOptionLine::onPickColor));
        m_colpk1->setPosition(CCPoint(10.f, 10.f));
        m_colpk1->setID(mode == 4 ? "zero" : "grad1");
        m_colpk1->setTag(mode == 4 ? 4 : 2);
        this->addChild(m_colpk1);

        sqr = ColorChannelSprite::create();
        sqr->setScale(0.5);
        this->m_colpk2 = CCMenuItemSpriteExtra::create(sqr, this, menu_selector(SetupOptionLine::onPickColor));
        m_colpk2->setPosition(CCPoint(170.f, 10.f));
        m_colpk2->setID(mode == 4 ? "hdrd" : "grad2");
        m_colpk2->setTag(mode == 4 ? 5 : 3);
        this->addChild(m_colpk2);

        label = CCLabelBMFont::create(mode == 4 ? "0%" : "Color1", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentLeft);
        label->setScale(0.3);
        label->setPosition(CCPoint(25.f, 10.f));
        label->setAnchorPoint(CCPoint(0.f, 0.5));
        this->addChild(label);

        label = CCLabelBMFont::create(mode == 4 ? "100%" : "Color2", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentRight);
        label->setScale(0.3);
        label->setPosition(CCPoint(155.f, 10.f));
        label->setAnchorPoint(CCPoint(1.f, 0.5));
        this->addChild(label);
        break;
    case OptionLineType::Slider:
        this->max = mode == 3 ? 99 : 100;
        // label
        this->setID(mode == 3 ? "duty" :"satu");
        m_label = CCLabelBMFont::create(mode == 3 ? "Duty %" :"Saturation", "ErasBold.fnt"_spr, 140.f, CCTextAlignment::kCCTextAlignmentLeft);
        m_label->setPosition(CCPoint(10.f, 10.f));
        m_label->setAnchorPoint(CCPoint(0.f, 0.5));
        m_label->setScale(0.4);
        this->addChild(m_label);    

        // inputer
        m_inputer = TextInput::create(40.f, mode == 3 ? "duty" :"satu", "ErasBold.fnt"_spr);
        m_inputer->setPosition(CCPoint(200.f, 10.f));
        m_inputer->setScale(0.5);
        m_inputer->setFilter("1234567890");
        m_inputer->setDelegate(this);
        m_inputer->getChildByType<CCScale9Sprite>(0)->setVisible(false);
        m_inputer->setID("text-input");
        this->addChild(m_inputer);

        // arrow left
        auto arrowLeft = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
        arrowLeft->setScale(0.15);
        arrowLeft->setFlipX(true);
        m_btnLeft = CCMenuItemSpriteExtra::create(arrowLeft, this, menu_selector(SetupOptionLine::onArrow));
        m_btnLeft->setPosition(CCPoint(175.f, 10.f));
        m_btnLeft->setTag(1);
        this->addChild(m_btnLeft);

        // arrow right
        auto arrowRight = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
        arrowRight->setScale(0.15);
        m_btnRight = CCMenuItemSpriteExtra::create(arrowRight, this, menu_selector(SetupOptionLine::onArrow));
        m_btnRight->setPosition(CCPoint(225.f, 10.f));
        m_btnRight->setTag(2);
        this->addChild(m_btnRight);

        // slider
        m_slider = Slider::create(this, menu_selector(SetupOptionLine::onSlider), 0.4);
        m_slider->setPosition(CCPoint(120.f, 10.f));
        m_slider->m_delegate = this;
        m_slider->setID("slider");
        this->addChild(m_slider);

        break;
    }
    return true;
}

void SetupOptionLine::helpFade(bool in) {
    if (m_slider) {
        m_slider->getChildByType<CCSprite>(0)->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255*in)));
        m_slider->m_sliderBar->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255*in)));
        m_slider->getThumb()->getChildByTag(1)->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255*in)));
    }
    if (m_inputer)
        m_inputer->getChildByType<CCTextInputNode>(0)->getChildByType<CCLabelBMFont>(0)->runAction(
            CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255*in)));     
}

void SetupOptionLine::textChanged(CCTextInputNode* p) {
    std::string input = p->getString();
    if (input != "")
        m_slider->setValue(Val2Slider(stoi(input)));
}

void SetupOptionLine::textInputClosed(CCTextInputNode* p) {
    std::string input = p->getString();
    if (input == "")
        input = std::to_string(value);
    else {
        value = stoi(input);
        if (max > 0 && value > max)
            value = max;
        input = std::to_string(value);
    }
    p->setString(input);
    SignalEvent(mode == 3 ? "duty" :"satu", value).post();
}

void SetupOptionLine::onSlider(CCObject* sender) {
    value = Slider2Val(m_slider->getValue());
    m_inputer->setString(std::to_string(value));
}

void SetupOptionLine::onArrow(CCObject* sender) {

    float delta = sender->getTag() == 2 ? 0.1 : -0.1;
    float s = m_slider->getValue();
    s += delta;
    s = s > 1 ? 1 : (s < 0 ? 0 : s);
    
    value = Slider2Val(s);
    if (max > 0 && value > max) {
        value = max;
        m_slider->setValue(Val2Slider(max));
    } else
        m_slider->setValue(s);

    m_inputer->setString(std::to_string(value));
    SignalEvent(mode == 3 ? "duty" :"satu", value).post();
}

float MyContentLayer::getSomething(float Y, float H) {
    auto y = this->getPositionY() + Y;
    float p1 = ceilingHeight - y - H / 2;
    float p2 = y - H / 2;
    // invisible case
    if (p1 < 0 || p2 < 0)
        return 0;
    // visible case
    if (p1 > p2)
        return p2 > 25 ? 1 : 0.04*p2;
    else
        return p1 > 25 ? 1 : 0.04*p1;
}
/*
void ScrollLayerPlus::Transition(bool in, int move) {
    auto m_realCL = static_cast<MyContentLayer*>(m_contentLayer);
    // move to top plz
    if (in) {
        if (move)
            m_realCL->setPositionY(move > 5 ? 210.f - 40.f * move : 0.f);
        else
            this->moveToTop();
    }
    // animation        
    int m = this->getTag();
    int tag = in ? m : 1;
    float delay = 0;
    
    float X = this->m_contentLayer->getContentWidth() / 2;

    // fade buttons
    while (tag > 0 && tag <= m) {
        if (auto opt = static_cast<BaseCell*>(m_contentLayer->getChildByTag(tag))) {
            // casted contentLayer
            
            // start
            float y0 = m_realCL->Ystd.at(tag-1) - (in && move ? 0.f : 0);
            float tg0 = m_realCL->getSomething(y0, opt->getContentHeight());
            // dest
            float y1 = m_realCL->Ystd.at(tag-1) - (in || !move ? 0 : 0.f);
            float tg1 = m_realCL->getSomething(y1, opt->getContentHeight());
            opt->stopInnerAction();

            //log::warn("start tag = {} y0 = {} tg0 = {} y1 = {} tg1 = {}", tag, y0, tg0, y1, tg1);
            // we won't see them at all
            if ((in && !tg1) || (!in && !tg0)) {
                // place it in the dest status
                opt->setPosition(CCPoint(X, y1));
                opt->setVisible(false);
                opt->setScale(0.5);
                opt->setOpacity(0);
                tag += in ? -1 : 1;
                continue;
            }
            // set it in the start status
            opt->setPosition(CCPoint(X, y0));
            opt->setVisible(true);
            opt->setScale(in ? 0.25 + 0.25 * tg0 : 0.5 + 0.5 * tg0);
            opt->setOpacity(in ? 0 : 255 * tg0);

            float newScale = in ? 0.5 + 0.5 * tg1 : 0.25 + 0.25 * tg1;

            auto actIn = CCSequence::create(
                CCDelayTime::create(delay),
                CCSpawn::create(
                    CCEaseExponentialOut::create(CCScaleTo::create(ANIM_TIME_M, 0.5 + 0.5 * tg1)),
                    CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255 * tg1)),
                    CCEaseExponentialOut::create(CCMoveTo::create(ANIM_TIME_M, CCPoint(X, y1))),
                    nullptr
                ),
                CallFuncExt::create([opt, tg1] () {
                    opt->setVisible(true);
                    opt->setOpacity(255 * tg1);
                }),
                nullptr
            );
            auto actOut = CCSequence::create(
                CCDelayTime::create(delay),
                CCSpawn::create(
                    CCEaseExponentialOut::create(CCScaleTo::create(ANIM_TIME_M, tg1 > tg0 ? 0.25 + 0.25 * tg0 : 0.25 + 0.25 * tg1)),
                    CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 0)),
                    CCEaseExponentialOut::create(CCMoveTo::create(ANIM_TIME_M, CCPoint(X, y1))),
                    nullptr
                ),
                CallFuncExt::create([opt] () {
                    opt->setVisible(false);
                    opt->setOpacity(0);
                }),
                nullptr
            );
            opt->runActionPlus(in ? actIn : actOut);

            tag += in ? -1 : 1;
            delay += ANIM_TIME_GAP;
        } else
            break;
    }
    // stop former fade to avoid spam issue
    if (actionFade)
        this->stopAction(actionFade);
    if (in)
        this->setVisible(true);
    else {
        actionFade = CCSequence::create(
            CCDelayTime::create(delay + ANIM_TIME_M),
            CallFuncExt::create([this](void) { this->setVisible(false); }),
            nullptr
        );
        this->runAction(actionFade);
    }
}
*/

void ScrollLayerPlus::Transition(bool in, int move) {
    auto m_realCL = static_cast<MyContentLayer*>(m_contentLayer);
    // move to top plz
    if (in) {
        if (move)
            m_realCL->setPositionY(move > 5 ? 210.f - 40.f * move : 0.f);
        else
            this->moveToTop();
    }
    // animation        
    int m = this->getTag();
    int tag = in ? m : 1;
    float delay = 0;
    
    float X = this->m_contentLayer->getContentWidth() / 2;

    // fade buttons
    while (tag > 0 && tag <= m) {
        if (auto opt = static_cast<CCMenu*>(m_contentLayer->getChildByTag(tag))) {
            // casted contentLayer
            
            // start
            float y0 = m_realCL->Ystd.at(tag-1) - (in && !move ? 0.f : 0);
            float tg0 = m_realCL->getSomething(y0, opt->getContentHeight());
            // dest
            float y1 = m_realCL->Ystd.at(tag-1) - (in || move ? 0 : 0.f);
            float tg1 = m_realCL->getSomething(y1, opt->getContentHeight());
            //opt->stopInnerAction();

            //log::warn("start tag = {} y0 = {} tg0 = {} y1 = {} tg1 = {}", tag, y0, tg0, y1, tg1);
            // we won't see them at all
            if ((in && !tg1) || (!in && !tg0)) {
                // place it in the dest status
                opt->setPosition(CCPoint(X, y1));
                opt->setVisible(false);
                opt->setScale(0.5);
                opt->setOpacity(0);
                tag += in ? -1 : 1;
                continue;
            }
            // set it in the start status
            opt->setPosition(CCPoint(X, y0));
            opt->setVisible(true);
            opt->setScale(in ? 0.25 + 0.25 * tg0 : 0.5 + 0.5 * tg0);
            opt->setOpacity(in ? 0 : 255 * tg0);

            //float newScale = in ? 0.5 + 0.5 * tg1 : 0.25 + 0.25 * tg1;

            if (in)
                m_realCL->acts.at(tag-1) = opt->runAction(CCSequence::create(
                    CCDelayTime::create(delay),
                    CCSpawn::create(
                        CCEaseExponentialOut::create(CCScaleTo::create(ANIM_TIME_M, 0.5 + 0.5 * tg1)),
                        CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255 * tg1)),
                        CCEaseExponentialOut::create(CCMoveTo::create(ANIM_TIME_M, CCPoint(X, y1))),
                        nullptr
                    ),
                    CallFuncExt::create([opt, tg1] () {
                        opt->setVisible(true);
                        opt->setOpacity(255 * tg1);
                    }),
                    CCDelayTime::create(ANIM_TIME_L),
                    nullptr
                ));
            else
                m_realCL->acts.at(tag-1) = opt->runAction(CCSequence::create(
                    CCDelayTime::create(delay),
                    CCSpawn::create(
                        CCEaseExponentialOut::create(CCScaleTo::create(ANIM_TIME_M, tg1 > tg0 ? 0.25 + 0.25 * tg0 : 0.25 + 0.25 * tg1)),
                        CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 0)),
                        CCEaseExponentialOut::create(CCMoveTo::create(ANIM_TIME_M, CCPoint(X, y1))),
                        nullptr
                    ),
                    CallFuncExt::create([opt] () {
                        opt->setVisible(false);
                        opt->setOpacity(0);
                    }),
                    CCDelayTime::create(ANIM_TIME_L),
                    nullptr
                ));

            delay += ANIM_TIME_GAP;
            tag += in ? -1 : 1;
            //delay += ANIM_TIME_GAP;
        } else
            break;
    }
    // stop former fade to avoid spam issue
    if (actionFade)
        this->stopAction(actionFade);
    if (in)
        this->setVisible(true);
    else {
        actionFade = CCSequence::create(
            CCDelayTime::create(delay + ANIM_TIME_M),
            CallFuncExt::create([this](void) { this->setVisible(false); }),
            nullptr
        );
        this->runAction(actionFade);
    }
}