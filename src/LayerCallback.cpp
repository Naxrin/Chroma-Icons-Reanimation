// This file describes how the layer process direct callback from his buttons and signal from other menu classes inside.
#include "Layer.hpp"
#include <Geode/ui/GeodeUI.hpp>

extern std::map<short, ChromaSetup> setups;
extern std::map<std::string, bool> opts;
extern float speed;
// reset chroma
std::map<PlayerObject*, bool> reset;

/**************** EVENT HANDLER *******************/
ListenerResult ChromaLayer::handleBoolSignal(SignalEvent<bool>* event) {
    // activate
    if (event->name == "activate") {
        if (!reset.empty())
            for (auto [key, _] : reset)
                reset[key] = true;

        // item menu toggle preview
        m_cellItemAdv->toggleChroma();
        m_cellItemEasy->toggleChroma();
        m_cellItemEffect->toggleChroma();

        // setup item
        for (auto cell : CCArrayExt<SetupItemCell*>(m_scrollerSetupTabsAdv->m_contentLayer->getChildren()))
            cell->toggleChroma(cell == m_currentTab);
        for (auto cell : CCArrayExt<SetupItemCell*>(m_scrollerSetupTabsEasy->m_contentLayer->getChildren()))
            cell->toggleChroma(cell == m_currentTab);
    }
    
    // switch
    else if (event->name == "prev") {
        if (opts["activate"]) {
            // toggle preview
            m_cellItemAdv->toggleChroma();
            m_cellItemEasy->toggleChroma();
            m_cellItemEffect->toggleChroma();
        }
    }

    // switch theme
    else if (event->name == "dark-theme")
        this->switchTheme();

    // blur bg switch
    else if (event->name == "blur-bg") {
        if (m_blur) {
            if (event->value) {
                this->m_blur->setVisible(true);
                this->m_blur->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_L, 255)));
                this->m_bg->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_L, 144)));
            } else {
                this->m_blur->runAction(CCFadeOut::create(ANIM_TIME_L));
                this->m_bg->runAction(CCFadeTo::create(ANIM_TIME_L, 196));
            }
        }
    }

    // check best toggler in workspace
    else if (event->name == "best") {
        m_currentSetup.best = event->value;
        this->dumpConfig();
    }


    // never allow on close when dragging slider
    else if (event->name == "drag-slider")
        this->m_onSlider = event->value;

    return ListenerResult::Stop;
}

ListenerResult ChromaLayer::handleIntSignal(SignalEvent<int>* event) {
    // drag speed slider
    if (event->name == "mode") {
        this->m_currentSetup.mode = event->value;
        this->m_cellWorkspace->refreshUI(m_currentSetup, true);
        this->dumpConfig();
    }

    // set duty value
    else if (event->name == "duty") {
        // cache
        ccColor3B grad1 = m_currentSetup.gradient.begin()->second;
        ccColor3B grad2 = m_currentSetup.gradient.rbegin()->second;
        // clear
        m_currentSetup.gradient.clear();
        // construct
        this->m_currentSetup.gradient[0] = grad1;
        int d = event->value ? (int)round(event->value * 18 / 5) : 1;
        this->m_currentSetup.gradient[d] = grad2;
        if (d > 180)
            this->m_currentSetup.gradient[2 * d - 360] = grad1;
        if (d < 180)
            this->m_currentSetup.gradient[360 - d] = grad2;
        this->dumpConfig();
    }

    // set saturation
    else if (event->name == "satu") {
        this->m_currentSetup.satu = event->value;
        this->dumpConfig();
    }

    // pick an item from item page or setup page
    else if (event->name == "pick") {
        // from item menu icon
        if (this->m_pages.back() == Page::Item) {
            if (!this->m_hasSetupPage)
                this->makeSetupPage();
            this->switchTab(event->value);
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
            bool really_changed = this->switchTab(event->value);
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
    }

    // launch color page
    else if (event->name == "color") {
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
        this->m_colorTag = event->value;
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
    }

    // R cell
    else if (event->name == "color-R") {
        m_crtColor.r = event->value;
        this->refreshColorPage(1);
    }
    // G cell
    else if (event->name == "color-G") {
        m_crtColor.g = event->value;
        this->refreshColorPage(1);
    }
    // B cell
    else if (event->name == "color-B") {
        m_crtColor.b = event->value;
        this->refreshColorPage(1);
    }

    // warn page feedback
    else if (event->name == "warning")
        switch (event->value) {
        // never remind
        case 2:
            Mod::get()->setSavedValue("notify", false);
        // remind next time
        case 1:
            m_pages.pop_back();
            // leave warn page
            this->fadeWarnPage();
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
}

ListenerResult ChromaLayer::handleFloatSignal(SignalEvent<float>* event) {
    // drag speed slider
    if (event->name == "speed") {
        Mod::get()->setSavedValue("speed", event->value);
        speed = event->value;
    }
    return ListenerResult::Stop;
}

ListenerResult ChromaLayer::handleColorSignal(SignalEvent<ccColor3B>* event) {
    // drag speed slider
    if (event->name == "color-hex") {
        m_crtColor = event->value;
        this->refreshColorPage(2);
    }
    return ListenerResult::Stop;
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
                opts["easy"] ? "You are now in Easy Mode, \"Icon\" means your current config will work regardless of your player icons' gamemode."
                    : fmt::format("Your current modifying chroma pattern will only affect your player icon when she is in {} mode.",
                    items[(int)this->m_gamemode])
            );
            break;
        // channel hint
        case 4:
            this->makeHintPopup(
                "Channel Indicator",
                "       The target icon sprite part or effect you're currently modifying for current gamemode.\n"
                "Main / Second / Glow -> Robtop's official paint area\n"
                "White -> Extra detail sprite that keeps white in original games\n"
                "Also note that:\n"
                "       1.Ghost Trail is overwritten fixed and may not perfectly match RobTop's work. "
                "You can turn off my fix but ghost trail chroma will not work then.\n"
                "       2.Rainbow and fire Regular Trail will neither get rendered by RobTop nor Chroma Icons.\n"
                "       3.Spider TP line is also fixed, this fix is optional but here you can keep tp line chroma with this fix OFF."
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
    Task<bool> task;
    switch (sender->getTag()) {
    case 0:
        task = geode::openInfoPopup(Mod::get()->getID());
        return;
    case 1:
        system("start https://github.com/Naxrin/Chroma-Icons-Reanimation");
        return;
    case 10:
        system("start https://www.youtube.com/@Naxrin");
        return;
    case 11:
        system("start https://x.com/Naxrin19");
        return;
    case 12:
        system("start https://discordapp.com/users/414986613962309633");
        return;
    case 13:
        system("start https://space.bilibili.com/25982878");
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
        // blur
        if (m_blur)
            m_blur->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_L, 0)));
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
        // blur
        if (m_blur)
            m_blur->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_L, 0)));
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