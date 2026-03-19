#include "mynode.hpp"

extern std::map<short, ChromaPattern> setups;
extern std::map<std::string, bool> opts;
extern std::map<std::string, float> vals;

// UnlockType tags by RobTop
const static int gametype[9] = {1, 4, 5, 6, 7, 8, 9, 13, 14};
// IconType name for More Icons
const static std::string MInames[9] = {"icon", "ship", "ball", "ufo", "wave", "robot", "spider", "swing", "jetpack"};
// IconType and Colors name for Separate Dual Icons
const static std::string SDInames[9] = {"cube", "ship", "roll", "bird", "dart", "robot", "spider", "swing", "jetpack"};
//player mode
inline int garageIconIndex(int index) {
    switch (index) {
    case 8: return gm->getPlayerJetpack();
    case 7: return gm->getPlayerSwing();
    case 6: return gm->getPlayerSpider();
    case 5: return gm->getPlayerRobot();
    case 4: return gm->getPlayerDart();
    case 3: return gm->getPlayerBird();
    case 2: return gm->getPlayerBall();
    case 1: return gm->getPlayerShip();
    default: return gm->getPlayerFrame();
    }
}

bool GJItemEffect::init(int tab) {
    if (!CCSprite::initWithFile("effect_base.png"_spr))
        return false;
    // cascade opacity
    this->setCascadeOpacityEnabled(true);
    this->m_channel = Channel(tab - 6);
    // add cover
    std::string str = fmt::format("{}_{}.png","effect"_spr , tab - 10);
    this->m_cover = CCSprite::create(str.c_str());
    auto size = this->getContentSize();
    this->m_cover->setPosition(CCPoint(size.width / 2, size.height / 2));
    this->m_cover->setID("cover");
    this->addChild(this->m_cover);

    return true;
}

bool PickItemButton::init(int tag, bool src, CCObject* target, cocos2d::SEL_MenuHandler callback) {
    this->setTag(tag);
    this->m_src = src;

    // effect
    if (tag > 9) {
        // spr
        this->m_effect = GJItemEffect::createEffectItem(tag);
        this->m_effect->setScale(src ? 0.75 : 0.85);
        // init frame color
        this->setPlayerStatus();
        // why did I write such a line then?
        //this->m_effect->type() = Channel(tag - 6);
        // init button
        return CCMenuItemSpriteExtra::init(this->m_effect, this->m_effect, target, callback);
    }
    // icon
    // spr
    int index = tag ? tag - 1 : 0;
    this->m_icon = GJItemIcon::createBrowserItem(UnlockType(gametype[index]), garageIconIndex(index));
    this->m_player = static_cast<SimplePlayer*>(this->m_icon->m_player);
    // add default glow color
    if (gm->m_playerGlow)
        m_player->setGlowOutline(ccc3(255, 255, 255));
    // setup page
    if (src)
        m_icon->setScale(0.6);
    // adv mode item cell
    else if (tag)
        m_icon->setScale(0.8);
    // easy mode item cell
    else
        m_icon->setScale(1.15);

    // init frame and color
    this->setPlayerStatus();

    // cascade opacity ohh fuck
    this->m_icon->setCascadeOpacityEnabled(true);
    this->m_icon->m_player->setCascadeOpacityEnabled(true);
    for (CCObject* obj: CCArrayExt<CCObject*>(this->m_icon->m_player->getChildren()))
        static_cast<CCSprite*>(obj)->setCascadeOpacityEnabled(true);
    if (index == 5 || index == 6) {
        auto part = this->m_icon->m_player->getChildByType<CCAnimatedSprite>(0)->getChildByType<CCPartAnimSprite>(0);
        part->setCascadeOpacityEnabled(true);
        for (CCObject* obj: CCArrayExt<CCObject*>(part->getChildren()))
            static_cast<CCSprite*>(obj)->setCascadeOpacityEnabled(true);
    }
    // init button
    return CCMenuItemSpriteExtra::init(this->m_icon, this->m_icon, target, callback);
}

void PickItemButton::switchPlayer() {
    this->m_ptwo = !this->m_ptwo;
    setPlayerStatus();
}

void PickItemButton::setPlayerStatus() {
    int index = this->getTag() ? this->getTag() - 1 : 0;
    int f, m, s, g;
    
    // Sep Dual Icons compatible
    auto SDI = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
    // frame
    if (index < 9) {
        // update frames 
        f = (SDI && m_ptwo) ? SDI->getSavedValue<int64_t>(SDInames[index]) : garageIconIndex(index);
        m_player->updatePlayerFrame(f, IconType(index));
        if (index == 5)
            this->m_player->m_robotSprite->updateFrame(f);
        if (index == 6)
            this->m_player->m_spiderSprite->updateFrame(f);
            
        // More Icons compatible
        if (auto MI = Loader::get()->getLoadedMod("hiimjustin000.more_icons")) {
            // config
            auto name = MI->getSavedValue<std::string>(MInames[index] + (this->m_ptwo ? "-dual" : ""));
            // post event
            /*
            Dispatch<SimplePlayer*, std::string, IconType>(
                "hiimjustin000.more_icons/simple-player", player, name, IconType(index)).post();*/
        }           
    }
    if (SDI && this->m_ptwo) {
        // colors
        m = SDI->getSavedValue<int64_t>("color1");
        s = SDI->getSavedValue<int64_t>("color2");
        g = SDI->getSavedValue<int64_t>("colorglow");
    } else {
        m = this->m_ptwo ? gm->getPlayerColor2() : gm->getPlayerColor();
        s = this->m_ptwo ? gm->getPlayerColor() : gm->getPlayerColor2();
        g = gm->getPlayerGlowColor();
    }

    this->m_mainColor = gm->colorForIdx(m);
    this->m_secondColor = gm->colorForIdx(s);
    this->m_glowColor = gm->colorForIdx(g);
}

void PickItemButton::delayFade(int delay, bool in) {
    this->runAction(CCSequence::create(
        CCDelayTime::create(0.01 + ANIM_TIME_GAP * (1 + delay)),
        CallFuncExt::create([this, in](void) { fade(this, in, ANIM_TIME_L); }),
        nullptr
    ));
}

void PickItemButton::runChroma(float const& phase, float const& percentage, int const& progress) {
    if (!this->m_chroma)
        return;
    if (this->getTag() > 9)
        this->m_effect->setColor(getChroma(
            setups[getIndex(this->m_ptwo, this->m_effect->getGamemode(), this->m_effect->getChannel())], this->m_mainColor, phase, percentage, progress));
    else {
        // main
        this->m_player->setColor(getChroma(
            setups[getIndex(this->m_ptwo, Gamemode(getTag()), Channel::Main)], this->m_mainColor, phase, percentage, progress));
        // second
        this->m_player->setSecondColor(getChroma(
            setups[getIndex(this->m_ptwo, Gamemode(getTag()), Channel::Secondary)], this->m_secondColor, phase + 120 * opts["sep-second"], percentage, progress));
        // glow
        if (this->m_player->m_hasGlowOutline)
            this->m_player->setGlowOutline(getChroma(
                setups[getIndex(this->m_ptwo, Gamemode(getTag()), Channel::Glow)], this->m_glowColor, phase + 240 * opts["sep-glow"], percentage, progress));

        // white
        auto white = getChroma(setups[getIndex(this->m_ptwo, Gamemode(getTag()), Channel::White)], ccc3(255, 255, 255), phase, percentage, progress);
        if (this->m_icon->m_unlockType == UnlockType::Robot)
            this->m_player->m_robotSprite->m_extraSprite->setColor(white);
        else if (this->m_icon->m_unlockType == UnlockType::Spider)
            this->m_player->m_spiderSprite->m_extraSprite->setColor(white);
        else
            this->m_player->m_detailSprite->setColor(white);
    }
}

void PickItemButton::toggleChroma() {
    this->toggleChroma(opts["activate"] && opts["prev"]);
}

// toggle on or off chroma, activated means not grey
void PickItemButton::toggleChroma(bool current) {
    // toggle
    this->m_chroma = current;
    // on
    if (this->m_chroma)
        return;
    // off
    if (this->getTag() > 9)
        this->m_effect->setColor(opts["activate"] ? this->m_mainColor : ccc3(127, 127, 127));
    else {
        this->m_player->setColor(opts["activate"] ? this->m_mainColor : ccc3(175, 175, 175));
        this->m_player->setSecondColor(opts["activate"] ? this->m_secondColor : ccc3(255, 255, 255));
        if (this->m_player->m_hasGlowOutline)
            this->m_player->setGlowOutline(opts["activate"] ? this->m_glowColor : ccc3(255, 255, 255));

        // white
        if (this->m_icon->m_unlockType == UnlockType::Robot)
            this->m_player->m_robotSprite->m_extraSprite->setColor(ccc3(255, 255, 255));
        else if (this->m_icon->m_unlockType == UnlockType::Spider)
            this->m_player->m_spiderSprite->m_extraSprite->setColor(ccc3(255, 255, 255));
        else if (this->m_player->m_detailSprite)
            this->m_player->m_detailSprite->setColor(ccc3(255, 255, 255));
    }
}

void PickItemButton::setModeTarget(Gamemode gamemode) {
    if (this->m_effect->getChannel() == Channel::WaveTrail && (int)gamemode) {
        this->m_effect->setGamemode(Gamemode::Wave);
        return;
    }
    if (this->m_effect->getChannel() == Channel::UFOShell && (int)gamemode) {
        this->m_effect->setGamemode(Gamemode::Ufo);
        return;
    }
    this->m_effect->setGamemode(gamemode);
}

bool SliderBundleBase::init(std::string topic, const char* title, float value, float max, float min, int precision, bool has_arrow, \
    float labelScale, float sliderScale, float inputerScale, float arrowScale, float sliderPosX, float inputerPosX, float labelWidth, float inputerWidth, float arrowDistance,\
    std::function<float (float)> toSlider, std::function<float (float)> fromSlider) {

    this->m_topic = topic;

    this->m_max = max;
    this->m_min = min;
    this->m_precision = precision;

    this->m_toSlider = toSlider;
    this->m_fromSlider = fromSlider;

    this->ignoreAnchorPointForPosition(false);
    this->setVisible(false);
    // label
    this->m_label = CCLabelBMFont::create(title, "ErasBold.fnt"_spr, labelWidth, CCTextAlignment::kCCTextAlignmentLeft);
    this->m_label->setPosition(CCPoint(0.f, 10.f));
    this->m_label->setAnchorPoint(CCPoint(0.f, 0.5));
    this->m_label->setScale(labelScale);
    this->addChild(this->m_label);

    // inputer
    this->m_inputer = TextInput::create(inputerWidth, title, "ErasBold.fnt"_spr);
    this->m_inputer->setPosition(CCPoint(inputerPosX, 10.f));
    this->m_inputer->setScale(inputerScale);
    this->m_inputer->setFilter(fmt::format("1234567890{}{}", this->m_precision ? "." : "", this->m_min < 0 ? "-" : ""));
    this->m_inputer->setDelegate(this);
    //m_inputer->getChildByType<CCScale9Sprite>(0)->setVisible(false);
    this->m_inputer->setID("text-input");
    this->addChild(this->m_inputer);

    // slider
    this->m_slider = Slider::create(this, menu_selector(SliderBundleBase::onSlider), sliderScale);
    this->m_slider->setPosition(CCPoint(sliderPosX, 10.f));
    this->m_slider->m_delegate = this;
    this->addChild(this->m_slider);

    if (has_arrow) {
        // arrow left
        auto sprLeft = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
        sprLeft->setScale(arrowScale);
        sprLeft->setFlipX(true);
        this->m_btnLeft = CCMenuItemSpriteExtra::create(sprLeft, this, menu_selector(SliderBundleBase::onArrow));
        this->m_btnLeft->setPosition(CCPoint(inputerPosX - arrowDistance, 10.f));
        this->m_btnLeft->setTag(1);
        this->addChild(this->m_btnLeft);

        // arrow right
        auto sprRight = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
        sprRight->setScale(arrowScale);
        this->m_btnRight = CCMenuItemSpriteExtra::create(sprRight, this, menu_selector(SliderBundleBase::onArrow));
        this->m_btnRight->setPosition(CCPoint(inputerPosX + arrowDistance, 10.f));
        this->m_btnRight->setTag(2);
        this->addChild(this->m_btnRight);        
    }

    // init value
    this->setVal(value);
    return true;
}

void SliderBundleBase::setVal(float value, short mode) {
    if (!mode)
        this->m_value = value;
    if (mode < 1 && m_inputer)
        // inputer
        m_inputer->setString(numToString(value, this->m_precision));
    if (mode > -1 && m_slider)
        // slider
        this->m_slider->setValue(std::clamp(this->m_toSlider(value), 0.f, 1.f));
}

void SliderBundleBase::helpFade(bool in) {
    if (m_slider != nullptr)
        fadeSlider(m_slider, in);
    if (m_inputer != nullptr)
        m_inputer->getChildByType<CCTextInputNode>(0)->getChildByType<CCLabelBMFont>(0)->runAction(
            CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_M, 255*in)));
}

bool SpeedSliderBundle::init() {
    if (!CCMenu::init())
        return false;
    if (!SliderBundleBase::init("speed", "Frequency", Mod::get()->getSavedValue<float>("speed", 1), 360, 0, 2, true,
        0.6, 0.6, 0.8, 0.3, 160.f, 275.f, 140.f, 60.f, 35.f,
        [](float value) -> float { return std::clamp(sqrt(value/5), 0.f, 1.f); },
        [](float s) -> float { return 5 * s * s; }
    )) return false;

    this->setID("speed-menu");
    this->setPosition(CCPoint(0, -120.f));
    this->setAnchorPoint(CCPoint(0.5, 0.5));
    this->setContentSize(CCSize(320.f, 20.f));
    this->setScale(5, 0.2);
    return true;
}

void SpeedSliderBundle::Fade(bool in) {
    fade(this, in, ANIM_TIME_M, in ? 1 : 5, in ? 1 : 0.2);
    // manually cascade opacity
    this->helpFade(in);
}

const char* titles[5] = {"Default", "Static", "Chromatic", "Gradient", "Progress"};
std::string descs[5] = {
    "As if this mod isn't loaded here.",
    "Set to any color you prefer.",
    "Our favourite Hue Cycle Mode.  \n",
    //"- Phase option gives an alter offset besides <cy>Separate Dual Phase</c> and so on. \n"
    //"- Set saturation percent to 50 if you want pastel like icons. \n"
    //"- Do you really need Brightness slider?",
    "Gradient between two colors",
    "Let this color gradient from one to another regarding your percentage or progress,"
    "In plat levels your current progress is always regarded 0."
};

bool SetupOptionLine::init(OptionLineType type, int mode, int tag) {
    if (!CCMenu::init())
        return false;

    this->m_type = type;
    this->m_mode = mode;
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
        // special height
        this->setContentHeight(25.f);

        this->m_title = CCLabelBMFont::create(titles[mode], "ErasBold.fnt"_spr, 240.f, CCTextAlignment::kCCTextAlignmentLeft);
        this->m_title->setScale(0.5);
        this->m_title->setPosition(CCPoint(25.f, 10.f));
        this->m_title->setAnchorPoint(CCPoint(0.f, 0.5));
        this->addChild(this->m_title);

        this->m_toggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(SetupOptionLine::onToggle), 0.55);
        this->m_toggler->setPosition(CCPoint(10.f, 10.f));
        this->m_toggler->setCascadeOpacityEnabled(true);
        this->addChild(this->m_toggler);
        break;
    case OptionLineType::Toggler:
        label = CCLabelBMFont::create("Best Progress", "ErasBold.fnt"_spr, 220.f, CCTextAlignment::kCCTextAlignmentLeft);
        label->setScale(0.4);
        label->setPosition(CCPoint(25.f, 10.f));
        label->setAnchorPoint(CCPoint(0.f, 0.5));
        this->addChild(label);

        this->m_toggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(SetupOptionLine::onToggle), 0.5);
        this->m_toggler->setPosition(CCPoint(10.f, 10.f));
        this->m_toggler->setCascadeOpacityEnabled(true);        
        this->addChild(this->m_toggler);
        break;
    case OptionLineType::Desc:
        label = CCLabelBMFont::create(descs[mode].c_str(), "ErasLight.fnt"_spr, 220.f, CCTextAlignment::kCCTextAlignmentLeft);
        label->setScale(0.8);
        label->setPosition(CCPoint(10.f, 10.f));
        label->setAnchorPoint(CCPoint(0.f, 0.5));
        label->setColor(ccc3(255, 255, 0));
        this->addChild(label);

        this->setContentHeight(label->getContentHeight());
        break;
    case OptionLineType::Color:
        sqr = ColorChannelSprite::create();
        sqr->setScale(0.5);
        this->m_colpk = CCMenuItemSpriteExtra::create(sqr, this, menu_selector(SetupOptionLine::onPickColor));
        this->m_colpk->setPosition(CCPoint(10.f, 10.f));
        this->m_colpk->setID("static");
        this->m_colpk->setTag(1);
        this->addChild(this->m_colpk);

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
        this->m_colpk2->setPosition(CCPoint(170.f, 10.f));
        this->m_colpk2->setID(mode == 4 ? "hdrd" : "grad2");
        this->m_colpk2->setTag(mode == 4 ? 5 : 3);
        this->addChild(this->m_colpk2);

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
        this->setID(mode == 3 ? "duty" :"satu");
        if (mode == 3)
            return SliderBundleBase::init("duty", "Duty %", 50, 99, 0, 0, true,
                0.4, 0.4, 0.5, 0.2, 120.f, 200.f, 140.f, 40.f, 25.f,
                [](float value) -> float { return value / 99; },
                [](float s) -> float { return 99 * s; }
            );
        else
            return SliderBundleBase::init("satu", "Saturation", 50, 100, 0, 0, true,
                0.4, 0.4, 0.5, 0.2, 120.f, 200.f, 140.f, 40.f, 25.f,
                [](float value) -> float { return value / 100; },
                [](float s) -> float { return 100 * s; }
            );
        break;
    }
    return true;
}

void SetupOptionLine::toggleTitle(bool yes, bool fade) {
    if (this->m_toggler)
        this->m_toggler->toggle(yes && !fade);
    if (this->m_type == OptionLineType::Title && this->m_title)
        // green or gray
        this->m_title->runAction(CCTintTo::create(fade * ANIM_TIME_M, 127-127*yes, 127+128*yes, 127-127*yes));
}

float MyContentLayer::getSomething(float Y, float H) {
    auto y = this->getPositionY() + Y;
    float p1 = this->m_ceilingHeight - y - H / 2;
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

void ScrollLayerPlus::Transition(bool in, int move) {
    auto realCL = static_cast<MyContentLayer*>(m_contentLayer);
    // move to top plz
    if (in) {
        if (move)
            // if move <= 5, the scroll layer should be at bottom already
            realCL->setPositionY(move > 5 ? 210.f - 40.f * move : 0.f);
        else
            // options
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
            // start
            float y0 = realCL->m_Ystd.at(tag-1) - (in && !move ? 0.f : 0);
            float tg0 = realCL->getSomething(y0, opt->getContentHeight());
            // dest
            float y1 = realCL->m_Ystd.at(tag-1) - (in || move ? 0 : 0.f);
            float tg1 = realCL->getSomething(y1, opt->getContentHeight());
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
                realCL->m_acts.at(tag-1) = opt->runAction(CCSequence::create(
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
                realCL->m_acts.at(tag-1) = opt->runAction(CCSequence::create(
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
    if (this->m_actionFade)
        this->stopAction(this->m_actionFade);
    if (in)
        this->setVisible(true);
    else {
        this->m_actionFade = CCSequence::create(
            CCDelayTime::create(delay + ANIM_TIME_M),
            CallFuncExt::create([this](void) { this->setVisible(false); }),
            nullptr
        );
        this->runAction(this->m_actionFade);
    }
}