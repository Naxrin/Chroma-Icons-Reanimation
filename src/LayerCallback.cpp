// This file describes how the layer process direct callback from his buttons and signal from other menu classes inside.
#include "Layer.hpp"
#include <Geode/ui/GeodeUI.hpp>

extern std::map<short, ChromaSetup> setups;
extern std::map<std::string, bool> opts;
extern float speed;
// reset chroma
std::map<PlayerObject*, bool> reset;

/**************** EVENT HANDLER *******************/

void ChromaLayer::installRadios() {
    this->m_radios.push_back(Signal<bool>("activate").listen([this] (bool activate) -> ListenerResult {
        if (!reset.empty())
            for (auto [key, _] : reset)
                reset[key] = true;

        // item menu toggle preview
        m_cellItemAdv->toggleChroma();
        m_cellItemEasy->toggleChroma();
        m_cellItemEffect->toggleChroma();

        // setup item
        if (m_hasSetupPage){
            for (auto cell : CCArrayExt<SetupItemCell*>(m_scrollerSetupTabsAdv->m_contentLayer->getChildren()))
                cell->toggleChroma(cell == m_currentTab);
            for (auto cell : CCArrayExt<SetupItemCell*>(m_scrollerSetupTabsEasy->m_contentLayer->getChildren()))
                cell->toggleChroma(cell == m_currentTab);            
        }

        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<bool>("prev").listen([this] (bool prev) -> ListenerResult {
        if (opts["activate"]) {
            // toggle preview
            m_cellItemAdv->toggleChroma();
            m_cellItemEasy->toggleChroma();
            m_cellItemEffect->toggleChroma();
        }

        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<bool>("dark-theme").listen([this] (bool dark) -> ListenerResult {
        this->switchTheme();
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<bool>("blur-bg").listen([this] (bool blur) -> ListenerResult {
        //BlurAPI::getOptions(this->m_bg)->passes = 1 + 4 * blur;
        m_bg->setVisible(blur);
        this->setOpacity(170 - 70 * blur);
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<bool>("drag-slider").listen([this] (bool drag) -> ListenerResult {
        this->m_onSlider = drag;
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<int>("pick").listen([this] (int tab) -> ListenerResult {
        // from item menu icon
        if (this->m_pages.back() == Page::Item) {
            if (!this->m_hasSetupPage)
                this->makeSetupPage();
            this->switchTab(tab);
            this->m_cellWorkspace->refreshUI(m_currentSetup, false);
            this->m_pages.push_back(Page::Setup);
            this->fadeItemPage();
            fade(m_btnInfo, false, ANIM_TIME_M);
            this->runAction(CCSequence::create(
                CCDelayTime::create(ANIM_TIME_M),
                CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeSetupPage)),
                CallFuncExt::create([this] () {
                    fade(m_btnInfo, m_pages.back() == Page::Item || m_pages.back() == Page::Options, ANIM_TIME_M);
                    fade(m_btnOptions, m_pages.back() == Page::Item || m_pages.back() == Page::Setup || m_pages.back() == Page::Color, ANIM_TIME_M);
                    fade(m_btnApply, m_pages.back() == Page::Setup || m_pages.back() == Page::Color, ANIM_TIME_M);
                }),
                nullptr
            ));
        }
        // from setup menu icon
        else if (this->m_pages.back() == Page::Setup) {
            bool really_changed = this->switchTab(tab);
            if (really_changed) {

                // show or hide channel switch arrow
                bool showArrows = this->m_tab < 14 && !opts["easy"] || !this->m_tab;
                fade(m_btnSetupArrowLeft, showArrows);
                fade(m_btnSetupArrowRight, showArrows);

                // workspace animation
                this->m_cellWorkspace->runAction(CCSequence::create(
                    CallFuncExt::create([this] () {
                        m_cellWorkspace->Fade(false);
                        m_cellWaitspace->refreshUI(m_currentSetup);
                        // swap pointers
                        auto temp = m_cellWaitspace;
                        m_cellWaitspace = m_cellWorkspace;
                        m_cellWorkspace = temp;
                    }),
                    CCDelayTime::create(ANIM_TIME_M / 3),
                    CallFuncExt::create([this] () {
                        m_cellWorkspace->Fade(true);
                        }),
                    nullptr
                ));          
            }
        }
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<int>("color").listen([this] (int tag) -> ListenerResult {
        if (!this->m_hasColorPage)
            this->makeColorPage();

        auto cur = m_pages.back();
        m_pages.push_back(Page::Color);
        // maybe planning more
        switch (cur) {
        case Page::Setup:
            this->fadeSetupPage();
            break;
        default:
            break;
        }
        // color page
        this->m_colorTag = tag;
        ccColor3B col = getColorTarget()->getColor();
        this->m_oriColor = col;
        this->m_crtColor = col;
        this->refreshColorPage(0);
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_M),
            CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeColorPage)),
            CallFuncExt::create([this] () {
                fade(m_btnInfo, m_pages.back() == Page::Item || m_pages.back() == Page::Options, ANIM_TIME_M);
                fade(m_btnOptions, m_pages.back() == Page::Item || m_pages.back() == Page::Setup || m_pages.back() == Page::Color, ANIM_TIME_M);
                fade(m_btnApply, m_pages.back() == Page::Setup || m_pages.back() == Page::Color, ANIM_TIME_M);
            }),
            nullptr
        ));
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<int>("warning").listen([this] (int warn) -> ListenerResult {
        switch (warn) {
        // never remind
        case 2:
            Mod::get()->setSavedValue("notified", true);
        // remind next time
        case 1:
            m_pages.pop_back();
            // leave warn page
            this->fadeWarnPage();
            // go to main menu
            m_pages.push_back(Page::Init);
            // fade in item page
            this->fadeMainMenu();
            // make up item page
            this->makeItemPage();
            // fade in item page
            this->fadeItemPage();
            break;
        // escape
        default:
            this->onClose(nullptr);
        }
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<int>("mode").listen([this] (int mode) -> ListenerResult {
        this->m_currentSetup.mode = mode;
        this->m_cellWorkspace->refreshUI(m_currentSetup, true);
        this->dumpConfig();
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<ccColor3B>("color-hex").listen([this] (ccColor3B color) -> ListenerResult {
        m_crtColor = color;
        this->refreshColorPage(2);
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<float>("speed").listen([this] (float speed) -> ListenerResult {
        Mod::get()->setSavedValue("speed", speed);
        //this->speed = speed;
        this->dumpConfig();
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<int>("duty").listen([this] (int duty) -> ListenerResult {
        // cache
        ccColor3B grad1 = m_currentSetup.gradient.begin()->second;
        ccColor3B grad2 = m_currentSetup.gradient.rbegin()->second;
        // clear
        m_currentSetup.gradient.clear();
        // construct
        this->m_currentSetup.gradient[0] = grad1;
        int d = duty ? (int)round(duty * 18 / 5) : 1;
        this->m_currentSetup.gradient[d] = grad2;
        if (d > 180)
            this->m_currentSetup.gradient[2 * d - 360] = grad1;
        if (d < 180)
            this->m_currentSetup.gradient[360 - d] = grad2;
        this->dumpConfig();
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<int>("satu").listen([this] (int satu) -> ListenerResult {
        this->m_currentSetup.satu = satu;
        this->dumpConfig();
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<int>("color-R").listen([this] (int r) -> ListenerResult {
        m_crtColor.r = r;
        this->refreshColorPage(1);
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<int>("color-G").listen([this] (int g) -> ListenerResult {
        m_crtColor.g = g;
        this->refreshColorPage(1);
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<int>("color-b").listen([this] (int b) -> ListenerResult {
        m_crtColor.r = b;
        this->refreshColorPage(b);
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<bool>("best").listen([this] (bool best) -> ListenerResult {
        m_currentSetup.best = best;
        this->dumpConfig();
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<std::pair<std::string, std::string>>("option-desc").listen([this] (std::pair<std::string, std::string> pair) -> ListenerResult {
        this->m_pages.push_back(Page::Popup);
        this->fadeOptionsPage();
        fade(m_btnInfo, false, ANIM_TIME_M);
        this->makeHintPopup(pair.first, pair.second, 80.f);
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_M),
            CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadePopupPage)),
            nullptr
        ));
        return ListenerResult::Stop;
    }));
}

/**************** DIRECT CALLBACK *******************/

// switch player
void ChromaLayer::onSwitchPlayer(CCObject* sender) {
    // save config
    if (this->m_pages.back() == Page::Setup)
        this->dumpConfig();
    // flip own p2 value
    this->m_ptwo = !this->m_ptwo;
    // flip his twin button
    if (sender->getTag() < 3)
        m_btnItemPlayer->toggle(!m_btnSetupPlayer->isToggled());
    else if (this->m_hasSetupPage)
        m_btnSetupPlayer->toggle(!m_btnItemPlayer->isToggled());

    // new config
    m_currentSetup = setups[getIndex(this->m_ptwo, this->m_gamemode, this->m_channel)];

    // item menu toggle preview
    m_cellItemAdv->switchPlayer();
    m_cellItemEasy->switchPlayer();
    m_cellItemEffect->switchPlayer();

    if (m_hasSetupPage) {
        // setup item
        for (auto cell : CCArrayExt<SetupItemCell*>(m_scrollerSetupTabsAdv->m_contentLayer->getChildren()))
            cell->switchPlayer();
        for (auto cell : CCArrayExt<SetupItemCell*>(m_scrollerSetupTabsEasy->m_contentLayer->getChildren()))
            cell->switchPlayer();
    }

    if (this->m_pages.back() == Page::Setup)
        // if currently in setup page, then should refresh UI
        this->m_cellWorkspace->runAction(CCSequence::create(
            CallFuncExt::create([this] () {
                m_cellWorkspace->Fade(false);
                m_cellWaitspace->refreshUI(m_currentSetup);
                auto temp = m_cellWaitspace;
                m_cellWaitspace = m_cellWorkspace;
                m_cellWorkspace = temp;
            }),
            CCDelayTime::create(ANIM_TIME_M / 3),
            CallFuncExt::create([this] () {
                m_cellWorkspace->Fade(true);
                }),
            nullptr
        ));
    else if (m_hasSetupPage) {
        m_cellWorkspace->refreshUI(m_currentSetup);
    }
}

void ChromaLayer::onShowPopup(CCObject* sender) {
    
    int tag = sender->getTag();

    auto cur = this->m_pages.back();
    this->m_pages.push_back(Page::Popup);
    switch (cur) {
    // from setup menu
    case Page::Setup:
        this->fadeSetupPage();
        fade(m_btnApply, false, ANIM_TIME_M);
        switch (tag) {
        // gamemode hint
        case 3:
            this->makeHintPopup(
                "Gamemode Indicator",
                opts["easy"] ? "You are now in Easy Mode, **\"Icon\"** means your current config will work regardless of your player icons' gamemode."
                    : fmt::format("Your current modifying chroma pattern will only affect your player icon when she is in **{}** mode.",
                    items[(int)this->m_gamemode]),
                30.f
            );
            break;
        // channel hint
        case 4:
            this->makeHintPopup(
                "Channel Indicator",
                "Channel Indicator means the target icon sprite part or effect you're currently modifying for current gamemode.\n"
                "- Main / Second / Glow -> Robtop's official paint area.\n"
                "- White -> Extra detail sprite that keeps white in original games.  \n"
                "**Also note that:**\n"
                "- Ghost Trail is overwritten fixed and may not perfectly match RobTop's work.\n"
                "- You can turn off my fix but ghost trail chroma will not work then.\n"
                "- Rainbow and Fire Regular Trail will neither get rendered by RobTop nor Chroma Icons.\n"
                "- Spider TP line is also fixed, this fix is optional but teleport line chroma **will not work** with this fix OFF.",
                160.f
            );
            break;
        }
    
    default:
        break;
    }

    // delay enter info page
    this->runAction(CCSequence::create(
        CCDelayTime::create(ANIM_TIME_M),
        CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadePopupPage)),
        CallFuncExt::create([this] () {
            fade(m_btnInfo, m_pages.back() == Page::Item || m_pages.back() == Page::Options, ANIM_TIME_M);
            fade(m_btnOptions, m_pages.back() == Page::Item || m_pages.back() == Page::Setup || m_pages.back() == Page::Color, ANIM_TIME_M);
            fade(m_btnApply, m_pages.back() == Page::Setup || m_pages.back() == Page::Color, ANIM_TIME_M);
        }),
        nullptr
    ));
}

// Easy Mode Switch
void ChromaLayer::onSwitchEasyAdv(CCObject* sender) {
    // flip
    opts["easy"] = !opts["easy"];
    // flip setting value
    Mod::get()->setSavedValue("easy", opts["easy"]);
    // gamemode
    if (opts["easy"]) {
        this->m_gamemodeAdv = this->m_gamemode;
        this->m_gamemode = Gamemode::Icon;
    } else
        this->m_gamemode = this->m_gamemodeAdv;

    // set color
    //m_modeBtn->setColor(opts["easy"] ? ccc3(127, 127, 255) : ccc3(255, 127, 127));
    // why ?
    m_btnMode->toggle(opts["easy"]);
    // advanced mode
    m_cellItemAdv->Fade(!opts["easy"]);
    fade(this->getChildByID("item-menu")->getChildByTag(6),
        !opts["easy"], ANIM_TIME_L, !opts["easy"] ? 0.7 : 0.35, !opts["easy"] ? 0.7 : 0.35);
    // easy mode
    m_cellItemEasy->Fade(opts["easy"]);
    fade(this->getChildByID("item-menu")->getChildByTag(5),
        opts["easy"], ANIM_TIME_L, opts["easy"] ? 0.7 : 0.35, opts["easy"] ? 0.7 : 0.35);

    // effect target label
    fade(this->getChildByID("item-menu")->getChildByTag(8),
        !opts["easy"], ANIM_TIME_L, !opts["easy"] ? 0.5 : 0.25, !opts["easy"] ? 0.5 : 0.25);

    // switch effect preview target in items menu
    m_cellItemEffect->setModeTarget(this->m_gamemode);

    auto btn = static_cast<CCMenuItemToggler*>(sender);
    // not knowing how to deal with spamming ui bug for now @_@
    btn->setEnabled(false);
    this->runAction(CCSequence::create(
        CCDelayTime::create(ANIM_TIME_M),
        CallFuncExt::create([btn] () { btn->setEnabled(true); }),
        nullptr
    ));
}

// on switch channel page
void ChromaLayer::onSwitchChannelPage(CCObject* sender) {
    // no spamming :(
    if (!(this->m_tab < 14 && !opts["easy"] || !this->m_tab))
        return;
    // dump settings
    this->dumpConfig();
    // get what to do
    int dir = sender->getTag() > 1 ? 1 : -1;

    // easy mode
    if (opts["easy"])
        this->m_channel = Channel((int(this->m_channel) + dir + 9) % 9);
    // in effects modify, switch gamemode
    else if (this->m_tab > 9) {
        this->m_gamemode = Gamemode((int(this->m_gamemode) + dir + 8) % 9 + 1);
        // refresh target of setup menu
        for (auto cell : CCArrayExt<SetupItemCell*>(m_scrollerSetupTabsAdv->m_contentLayer->getChildren()))
            if (!cell->setModeTarget(this->m_gamemode))
                break;
    }
    // in icons modify, switch channel
    else if (this->m_channel == Channel::Main && dir == -1)
        this->m_channel = this->m_gamemode == Gamemode::Wave ? Channel::WaveTrail : (this->m_gamemode == Gamemode::Ufo ? Channel::UFOShell : Channel::TPLine);
    else if (this->m_channel == Channel::TPLine && dir == 1)
        this->m_channel = this->m_gamemode == Gamemode::Wave ? Channel::WaveTrail : (this->m_gamemode == Gamemode::Ufo ? Channel::UFOShell : Channel::Main);
    else if (this->m_channel == Channel::WaveTrail)
        this->m_channel = dir > 0 ? Channel::Main : Channel::TPLine;
    else if (this->m_channel == Channel::UFOShell)
        this->m_channel = dir > 0 ? Channel::Main : Channel::TPLine;
    else
        this->m_channel = Channel(int(this->m_channel) + dir);

    // load new setup
    this->m_currentSetup = setups[getIndex(this->m_ptwo, this->m_gamemode, this->m_channel)];

    // gamemode label
    auto txtSetupGamemode = static_cast<CCLabelBMFont*>(this->m_btnSetupGamemode->getChildByID("label"));
    txtSetupGamemode->setString(items[(int)this->m_gamemode].c_str());
    txtSetupGamemode->setPositionX(0.24 * txtSetupGamemode->getContentWidth());
    this->m_btnSetupGamemode->setContentWidth(0.48 * txtSetupGamemode->getContentWidth());
    this->m_btnSetupGamemode->setPositionX(120.f - m_winSize.width / 2 + 0.24 * txtSetupGamemode->getContentWidth());

    // channel label
    auto txtSetupChannel = static_cast<CCLabelBMFont*>(this->m_btnSetupChannel->getChildByID("label"));
    txtSetupChannel->setString(chnls[(int)this->m_channel].c_str());
    txtSetupChannel->setPositionX(0.18 * txtSetupChannel->getContentWidth());
    this->m_btnSetupChannel->setContentWidth(0.36 * txtSetupChannel->getContentWidth());
    this->m_btnSetupChannel->setPositionX(57.f - m_winSize.width / 2 + 0.18 * txtSetupChannel->getContentWidth());

    // refresh target of setup menu
    for (auto cell : CCArrayExt<SetupItemCell*>(m_scrollerSetupTabsAdv->m_contentLayer->getChildren()))
        if (!cell->setModeTarget(this->m_gamemode))
            break;
            
    // workspace
    this->m_cellWorkspace->runAction(CCSequence::create(
        CallFuncExt::create([this, dir] () {m_cellWorkspace->Fade(false, dir);}),
        CCDelayTime::create(dir ? ANIM_TIME_M / 3 : 0),
        CallFuncExt::create([this, dir] () {
            m_cellWaitspace->refreshUI(m_currentSetup);
            m_cellWaitspace->Fade(true, dir);
            auto temp = m_cellWaitspace;
            m_cellWaitspace = m_cellWorkspace;
            m_cellWorkspace = temp;            
        }),
        nullptr
    ));
}

void ChromaLayer::onOptionsPage(CCObject*) {
    if (!this->m_hasOptionsPage)
        this->makeOptionsPage();

    auto cur = m_pages.back();
    m_pages.push_back(Page::Options);

    // fade out the old page
    fade(m_btnOptions, false, ANIM_TIME_M);
    switch (cur) {
    case Page::Item:
        this->fadeItemPage();
        fade(m_btnMode, false, ANIM_TIME_M);
        break;
    case Page::Setup:
        this->fadeSetupPage();
        fade(m_btnApply, false, ANIM_TIME_M);
        break;
    case Page::Color:
        this->fadeColorPage();
        fade(m_btnApply, false, ANIM_TIME_M);
        break;
    default:
        break;
    }
    this->runAction(CCSequence::create(
        CCDelayTime::create(ANIM_TIME_M),
        CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeOptionsPage)),
        CallFuncExt::create([this] () {
            fade(m_btnInfo, m_pages.back() == Page::Item || m_pages.back() == Page::Options, ANIM_TIME_M);
            fade(m_btnOptions, m_pages.back() == Page::Item || m_pages.back() == Page::Setup || m_pages.back() == Page::Color, ANIM_TIME_M);
            fade(m_btnApply, m_pages.back() == Page::Setup || m_pages.back() == Page::Color, ANIM_TIME_M);
        }),
        nullptr
    ));
}

void ChromaLayer::onInfoPage(CCObject*) {
    if (!this->m_hasInfoPage)
        this->makeInfoPage();
        
    auto cur = m_pages.back();
    m_pages.push_back(Page::Info);

    // fade out old page
    fade(m_btnInfo, false, ANIM_TIME_M);
    switch (cur) {
    case Page::Item:
        this->fadeItemPage();
        fade(m_btnOptions, false, ANIM_TIME_M);
        fade(m_btnMode, false, ANIM_TIME_M);
        break;
    case Page::Options:
        this->fadeOptionsPage();
    default:
        break;
    }
    // delay enter info page
    this->runAction(CCSequence::create(
        CCDelayTime::create(ANIM_TIME_M),
        CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeInfoPage)),
        CallFuncExt::create([this] () {
            fade(m_btnInfo, m_pages.back() == Page::Item || m_pages.back() == Page::Options, ANIM_TIME_M);
            fade(m_btnOptions, m_pages.back() == Page::Item || m_pages.back() == Page::Setup || m_pages.back() == Page::Color, ANIM_TIME_M);
            fade(m_btnApply, m_pages.back() == Page::Setup || m_pages.back() == Page::Color, ANIM_TIME_M);
        }),
        nullptr
    ));
}

void ChromaLayer::onInfoButtons(CCObject* sender) {
    
    switch (sender->getTag()) {
    case 0:
        geode::openInfoPopup(Mod::get()->getID());
        return;
    case 1:
        CCApplication::sharedApplication()->openURL("https://github.com/Naxrin/Chroma-Icons-Reanimation");
        return;
    case 10:
        CCApplication::sharedApplication()->openURL("https://www.youtube.com/@Naxrin");
        return;
    case 11:
        CCApplication::sharedApplication()->openURL("https://x.com/Naxrin19");
        return;
    case 12:
        CCApplication::sharedApplication()->openURL("https://discordapp.com/users/414986613962309633");
        return;
    case 13:
        CCApplication::sharedApplication()->openURL("https://space.bilibili.com/25982878");
        return;
    }
}

void ChromaLayer::onColorDisplayBtn(CCObject* sender) {
    int tag = sender->getTag();
    // ori
    if (tag != 4)
        this->transistColorBtn(false, tag == 1);
    // crt
    if (tag != 2)
        this->transistColorBtn(true, tag == 3);
}

void ChromaLayer::onCopy(CCObject* sender) {
    switch (sender->getTag()) {
    case 1:
        this->m_clipColor.first = true;
        this->m_clipColor.second = m_oriColor;
        this->transistColorBtn(false, false);
        return;
    case 3:
        this->m_clipColor.first = true;
        this->m_clipColor.second = m_crtColor;
        this->transistColorBtn(true, false);
        return;
    case 5:
        this->m_clipSetup.first = true;
        this->m_clipSetup.second = m_currentSetup;
    }
}

void ChromaLayer::onPaste(CCObject* sender) {
    if (sender->getTag() == 6) {
        if (!m_clipSetup.first)
            return;
        m_currentSetup = this->m_clipSetup.second;
        this->dumpConfig();
        this->m_cellWorkspace->runAction(CCSequence::create(
            CallFuncExt::create([this] () {
                m_cellWorkspace->Fade(false);
                m_cellWaitspace->refreshUI(m_currentSetup);
                auto temp = m_cellWaitspace;
                m_cellWaitspace = m_cellWorkspace;
                m_cellWorkspace = temp;
            }),
            CCDelayTime::create(ANIM_TIME_M / 3),
            CallFuncExt::create([this] () {
                m_cellWorkspace->Fade(true);
                }),
            nullptr
        ));
        return;
    }
    if (sender->getTag() != 2 || !m_clipColor.first)
        return;
    m_crtColor = this->m_clipColor.second;
    this->transistColorBtn(true, false);
    this->refreshColorPage(4);
    
}

void ChromaLayer::onResc(CCObject* sender) {
    m_crtColor = this->m_oriColor;
    this->transistColorBtn(false, false);
    this->refreshColorPage(4);
}

void ChromaLayer::onApply(CCObject* sender) {
    if (m_pages.back() == Page::Color) {
        // color target
        this->getColorTarget()->setColor(m_crtColor);
        // set setup value
        switch (m_colorTag) {
        case 1:
            m_currentSetup.color = m_crtColor;
            break;
        case 2:
            m_currentSetup.gradient.begin()->second = m_crtColor;
            break;
        case 3:
            m_currentSetup.gradient.rbegin()->second = m_crtColor;
            break;
        case 4:
            m_currentSetup.progress.begin()->second = m_crtColor;
            break;
        case 5:
            m_currentSetup.progress.rbegin()->second = m_crtColor;
            break;
        }
        this->dumpConfig();
    }
    // im speechlees of the delay fade design
    if (m_pages.back() != Page::Item)
        this->onClose(nullptr);
}

void ChromaLayer::onClose(CCObject* sender) {
    // mute on dragging slider
    if (this->m_onSlider || this->m_pages.back() == Page::Init)
        return;

    auto cur = m_pages.back();
    m_pages.pop_back();

    // fade out current page
    switch (cur) {
    case Page::Info:
        this->fadeInfoPage();
        break;
    case Page::Popup:
        this->fadePopupPage();
        break;
    case Page::Options:
        this->fadeOptionsPage();
        if (m_pages.back() != Page::Item)
            fade(m_btnInfo, false, ANIM_TIME_M);
        break;
    case Page::Color:
        this->fadeColorPage();
        break;
    case Page::Setup:
        this->fadeSetupPage();
        fade(m_btnApply, false, ANIM_TIME_M);
        // dump config
        this->dumpConfig();
        break;
    // byebye menu
    case Page::Item:
        m_pages.pop_back();
        m_pages.push_back(Page::Terminal);
        this->fadeItemPage();
        m_pages.pop_back();
        this->fadeMainMenu();
        this->runAction(CCFadeTo::create(ANIM_TIME_L, 0));
        m_bg->runAction(CCFadeTo::create(ANIM_TIME_L, 0));
        // delayed base close
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_L),
            CallFuncExt::create([this] () { Popup::onClose(nullptr); }),
            nullptr
        ));
        return;
    // escape
    case Page::Warn:
        this->fadeWarnPage();
        this->runAction(CCFadeTo::create(ANIM_TIME_L, 0));
        m_bg->runAction(CCFadeTo::create(ANIM_TIME_L, 0));
        this->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_L, 0)));
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_L),
            CallFuncExt::create([this] () { Popup::onClose(nullptr); }),
            nullptr
        ));
        return;
    default:
        break;
    }
    
    // fade in the new
    switch (m_pages.back()) {
    case Page::Options:
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_M),
            CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeOptionsPage)),
            nullptr
        ));
        break;
    case Page::Color:
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_M),
            CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeColorPage)),
            nullptr
        ));
        break;
    case Page::Setup:
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_M),
            CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeSetupPage)),
            nullptr
        ));
        break;
    case Page::Item:
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_M),
            CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeItemPage)),
            nullptr
        ));
        break;
    default:
        break;
    }
    this->runAction(CCSequence::create(
        CCDelayTime::create(ANIM_TIME_M),
        CallFuncExt::create([this] () {
            fade(m_btnInfo, m_pages.back() == Page::Item || m_pages.back() == Page::Options, ANIM_TIME_M);
            fade(m_btnOptions, m_pages.back() == Page::Item || m_pages.back() == Page::Setup || m_pages.back() == Page::Color, ANIM_TIME_M);
            fade(m_btnApply, m_pages.back() == Page::Setup || m_pages.back() == Page::Color, ANIM_TIME_M);
        }),
        nullptr
    ));
}