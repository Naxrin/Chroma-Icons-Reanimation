#include "mynode.hpp"

// IconType name from More Icons
// @warning ONLY used for More Icons name analyze
static std::string names[9] = {"icon", "ship", "ball", "ufo", "wave", "robot", "spider", "swing", "jetpack"};

GJItemIconAlpha* GJItemIconAlpha::createBrowserItem(int typeID, int ID) {
    auto icon = new GJItemIconAlpha();
    UnlockType type = UnlockType(gametype[typeID]);
    if (icon && icon->init(type, ID, ccc3(175, 175, 175), ccc3(255, 255, 255), true, true, true, ccc3(255, 255, 255))) {

        // more icons compatible
        auto name = Loader::get()->getLoadedMod("hiimjustin000.more_icons")->getSavedValue<std::string>(names[typeID]);
        DispatchEvent<SimplePlayer*, std::string, IconType>(
            "hiimjustin000.more_icons/simple-player",
            icon->m_player, name,
            IconType(typeID)
        ).post();

        // cascade opacity ohh fuck
        icon->setCascadeOpacityEnabled(true);
        icon->m_player->setCascadeOpacityEnabled(true);
        for (CCObject* obj: CCArrayExt<CCObject*>(icon->m_player->getChildren()))
            static_cast<CCSprite*>(obj)->setCascadeOpacityEnabled(true);
        if (type == UnlockType::Robot || type == UnlockType::Spider) {
            auto part = icon->m_player->getChildByType<CCAnimatedSprite>(0)->getChildByType<CCPartAnimSprite>(0);
            part->setCascadeOpacityEnabled(true);
            for (CCObject* obj: CCArrayExt<CCObject*>(part->getChildren()))
                static_cast<CCSprite*>(obj)->setCascadeOpacityEnabled(true);
        }
        icon->autorelease();
        return icon;
    }
    CC_SAFE_DELETE(icon);
    return nullptr;
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
        // set color
        if (Loader::get()->isModLoaded("ninkaz.colorful-icons"))
            base->setColor(gm->colorForIdx(gm->getPlayerColor()));
        else
            base->setColor(ccc3(175, 175, 175));

        base->autorelease();
        return base;
    }
    CC_SAFE_DELETE(base);
    return nullptr;
}

void PickItemButton::delayFade(int delay, bool in) {
    this->runAction(CCSequence::create(
        CCDelayTime::create(ANIM_TIME_GAP*(1+delay)),
        CallFuncExt::create([this, in](void) {
            float s = in ? 1 : 0.5;
            fade(this, in, ANIM_TIME_L, s, s);
        }),
        nullptr
    ));        
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
    /*
    if (!m_slider) {
        log::info("where is your slider");
        if (!m_inputer)
            log::info("where is your inputer");
        return;
    }
    if (!m_inputer) {
        log::info("where is your inputer");
        return;
    }*/

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
        label = CCLabelBMFont::create("Best Progress", "ErasBold.fnt"_spr, 240.f, CCTextAlignment::kCCTextAlignmentLeft);
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
        label = CCLabelBMFont::create(descs[mode], "ErasLight.fnt"_spr, 240.f, CCTextAlignment::kCCTextAlignmentLeft);
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
        this->max = 100;
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

void ScrollLayerPlus::Transition(bool in, bool scale) {
    // move to top plz
    if (in)
        this->moveToTop();
    // animation        
    int m = this->getTag();
    int tag = in ? m : 1;
    float delay = 0;

    float X = this->m_contentLayer->getContentWidth() / 2;
    // fade buttons
    while (tag > 0 && tag <= m) {
        if (auto opt = static_cast<CCMenu*>(m_contentLayer->getChildByTag(tag))) {
            // get some sneaky things
            auto m_realCL = static_cast<MyContentLayer*>(m_contentLayer);
            float y0 = m_realCL->Ystd.at(tag-1) - (in ? m_realCL->offset : 0);
            float y1 = m_realCL->Ystd.at(tag-1) - (in ? 0 : m_realCL->offset);
            float tg0 = m_realCL->getSomething(y0, opt->getContentHeight());
            float tg1 = m_realCL->getSomething(y1, opt->getContentHeight());
            // place it in the original status
            opt->setPosition(CCPoint(X, y0));
            opt->setVisible(tg0 > 0);
            if (scale)
                opt->setScale(in ? 0.25 + 0.25 * tg0 : 0.5 + 0.5 * tg0);
            opt->setOpacity(in ? 0 : 255 * tg0);

            auto act = m_realCL->actions.at(tag-1);
            if (act)
                opt->stopAction(act);
            act = CCSequence::create(
                CCDelayTime::create(delay),
                CallFuncExt::create([opt, in, X, tg1, y1, scale](void) {
                    if (scale) {
                        float newScale = in ? 0.5 + 0.5 * tg1 : 0.25 + 0.25 * tg1;
                        fade(opt, in, ANIM_TIME_M, newScale, newScale, in ? 255 * tg1 : 0);
                        opt->runAction(CCEaseExponentialOut::create(CCMoveTo::create(ANIM_TIME_M, CCPoint(X, y1))));                
                    }
                }),
                nullptr
            );
            opt->runAction(act);

            if (in)
                tag --;
            else
                tag ++;
            if ((in && tg1 > 0) || (!in && tg0 > 0))
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