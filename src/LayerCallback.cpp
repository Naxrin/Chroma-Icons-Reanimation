// This file describes how the layer process direct callback from his buttons and signal from other menu classes inside.
#include "Layer.hpp"
#include <Geode/ui/GeodeUI.hpp>

extern std::map<short, ChromaSetup> setups;
extern std::map<std::string, bool> opts;
extern float speed;
std::map<PlayerObject*, int> reset;

/**************** EVENT HANDLER *******************/
ListenerResult ChromaLayer::handleBoolSignal(SignalEvent<bool>* event) {
    // activate
    if (event->name == "activate") {
        opts["activate"] = event->value;
        if (!reset.empty())
            for (auto [key, _] : reset)
                reset[key] = 5 * (int)!event->value;

        // item menu toggle preview
        m_advBundleCell->toggleChroma();
        m_ezyBundleCell->toggleChroma();
        m_effBundleCell->toggleChroma();

        // setup item
        for (auto cell : CCArrayExt<SetupItemCell*>(m_setupAdvScroller->m_contentLayer->getChildren()))
            cell->toggleChroma(cell == m_currentItem);
        for (auto cell : CCArrayExt<SetupItemCell*>(m_setupEasyScroller->m_contentLayer->getChildren()))
            cell->toggleChroma(cell == m_currentItem);
    }

    // switch
    else if (event->name == "prev") {
        if (opts["activate"]) {
            // toggle preview
            m_advBundleCell->toggleChroma();
            m_ezyBundleCell->toggleChroma();
            m_effBundleCell->toggleChroma();
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
        currentSetup.best = event->value;
        this->dumpConfig();
    }


    // never allow on close when dragging slider
    else if (event->name == "drag-slider")
        this->on_slider = event->value;
    return ListenerResult::Stop;
}

ListenerResult ChromaLayer::handleIntSignal(SignalEvent<int>* event) {
    // drag speed slider
    if (event->name == "mode") {
        this->currentSetup.mode = event->value;
        this->m_workspace->refreshUI(currentSetup, true);
        this->dumpConfig();
    }

    // set duty value
    else if (event->name == "duty") {
        // cache
        ccColor3B grad1 = currentSetup.gradient.begin()->second;
        ccColor3B grad2 = currentSetup.gradient.rbegin()->second;
        // clear
        currentSetup.gradient.clear();
        // construct
        this->currentSetup.gradient[0] = grad1;
        int d = event->value ? (int)round(event->value * 18 / 5) : 1;
        this->currentSetup.gradient[d] = grad2;
        if (d > 180)
            this->currentSetup.gradient[2 * d - 360] = grad1;
        if (d < 180)
            this->currentSetup.gradient[360 - d] = grad2;
        this->dumpConfig();
    }

    // set saturation
    else if (event->name == "satu") {
        this->currentSetup.satu = event->value;
        this->dumpConfig();
    }

    // pick an item from item page or setup page
    else if (event->name == "pick") {
        // from item menu icon
        if (this->pages.back() == Page::Item) {
            this->switchCurrentItem(event->value);
            this->m_workspace->refreshUI(currentSetup, false);
            this->pages.push_back(Page::Setup);
            this->fadeItemPage();
            fade(m_infoBtn, false, ANIM_TIME_M);
            this->runAction(CCSequence::create(
                CCDelayTime::create(ANIM_TIME_M),
                CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeSetupPage)),
                CallFuncExt::create([this] () {
                    fade(m_infoBtn, pages.back() == Page::Item || pages.back() == Page::Options, ANIM_TIME_M);
                    fade(m_optionsBtn, pages.back() == Page::Item || pages.back() == Page::Setup || pages.back() == Page::Color, ANIM_TIME_M);
                    fade(m_applyBtn, pages.back() == Page::Setup || pages.back() == Page::Color, ANIM_TIME_M);
                }),
                nullptr
            ));            
        }
        // from setup menu icon
        else if (this->pages.back() == Page::Setup) {
            bool really_changed = this->switchCurrentItem(event->value);
            if (really_changed) {
                // show or hide channel switch arrow
                bool showArrows = this->tab < 14 && !opts["easy"] || !this->tab;
                fade(m_leftArrowSetupBtn, showArrows);
                fade(m_rightArrowSetupBtn, showArrows);
                // workspace animation
                this->m_workspace->runAction(CCSequence::create(
                    CallFuncExt::create([this] () {
                        m_workspace->Fade(false);
                        m_waitspace->refreshUI(currentSetup);
                        auto temp = m_waitspace;
                        m_waitspace = m_workspace;
                        m_workspace = temp;
                    }),
                    CCDelayTime::create(ANIM_TIME_M / 3),
                    CallFuncExt::create([this] () {
                        m_workspace->Fade(true);
                        }),
                    nullptr
                ));          
            }
        }
    }

    // launch color page
    else if (event->name == "color") {
        auto cur = pages.back();
        pages.push_back(Page::Color);
        // maybe planning more
        switch (cur) {
        case Page::Setup:
            this->fadeSetupPage();
            break;
        default:
            break;
        }
        // color page
        this->colorTag = event->value;
        ccColor3B col = getColorTarget()->getColor();
        this->oriColor = col;
        this->crtColor = col;
        this->refreshColorPage(0);
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_M),
            CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeColorPage)),
            CallFuncExt::create([this] () {
                fade(m_infoBtn, pages.back() == Page::Item || pages.back() == Page::Options, ANIM_TIME_M);
                fade(m_optionsBtn, pages.back() == Page::Item || pages.back() == Page::Setup || pages.back() == Page::Color, ANIM_TIME_M);
                fade(m_applyBtn, pages.back() == Page::Setup || pages.back() == Page::Color, ANIM_TIME_M);
            }),
            nullptr
        ));
    }

    // R cell
    else if (event->name == "color-R") {
        crtColor.r = event->value;
        this->refreshColorPage(1);
    }
    // G cell
    else if (event->name == "color-G") {
        crtColor.g = event->value;
        this->refreshColorPage(1);
    }
    // B cell
    else if (event->name == "color-B") {
        crtColor.b = event->value;
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
            pages.pop_back();
            this->fadeWarnPage();
            pages.push_back(Page::Init);
            this->fadeMainMenu();
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
    if (event->name == "speed")
        this->updateSpeedValue(event->value);
    return ListenerResult::Stop;
}

ListenerResult ChromaLayer::handleColorSignal(SignalEvent<ccColor3B>* event) {
    // drag speed slider
    if (event->name == "color-hex") {
        crtColor = event->value;
        this->refreshColorPage(2);
    }
    return ListenerResult::Stop;
}

/**************** DIRECT CALLBACK *******************/

// switch player
void ChromaLayer::onSwitchPlayer(CCObject* sender) {
    // save config
    if (this->pages.back() == Page::Setup)
        this->dumpConfig();
    // flip own p2 value
    this->ptwo = !this->ptwo;
    // flip his twin button
    if (sender->getTag() < 3)
        m_playerItemBtn->toggle(!m_playerSetupBtn->isToggled());
    else
        m_playerSetupBtn->toggle(!m_playerItemBtn->isToggled());

    // new config
    currentSetup = setups[getIndex(this->ptwo, this->gamemode, this->channel)];

    // item menu toggle preview
    m_advBundleCell->switchPlayer();
    m_ezyBundleCell->switchPlayer();
    m_effBundleCell->switchPlayer();

    // setup item
    for (auto cell : CCArrayExt<SetupItemCell*>(m_setupAdvScroller->m_contentLayer->getChildren()))
        cell->switchPlayer();
    for (auto cell : CCArrayExt<SetupItemCell*>(m_setupEasyScroller->m_contentLayer->getChildren()))
        cell->switchPlayer();

    // if outside setup page, then refresh ui and return
    if (this->pages.back() != Page::Setup) {
        m_workspace->refreshUI(currentSetup);
        return;
    }
    // if currently in setup page, then should refresh UI
    this->m_workspace->runAction(CCSequence::create(
        CallFuncExt::create([this] () {
            m_workspace->Fade(false);
            m_waitspace->refreshUI(currentSetup);
            auto temp = m_waitspace;
            m_waitspace = m_workspace;
            m_workspace = temp;
        }),
        CCDelayTime::create(ANIM_TIME_M / 3),
        CallFuncExt::create([this] () {
            m_workspace->Fade(true);
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
        this->history = this->gamemode;
        this->gamemode = Gamemode::Icon;
    } else
        this->gamemode = this->history;

    // set color
    //m_modeBtn->setColor(opts["easy"] ? ccc3(127, 127, 255) : ccc3(255, 127, 127));
    // why ?
    m_modeBtn->toggle(opts["easy"]);
    // full mode
    m_advBundleCell->Fade(!opts["easy"]);
    fade(this->getChildByID("item-menu")->getChildByTag(6),
        !opts["easy"], ANIM_TIME_L, !opts["easy"] ? 0.7 : 0.35, !opts["easy"] ? 0.7 : 0.35);
    // easy mode
    m_ezyBundleCell->Fade(opts["easy"]);
    fade(this->getChildByID("item-menu")->getChildByTag(5),
        opts["easy"], ANIM_TIME_L, opts["easy"] ? 0.7 : 0.35, opts["easy"] ? 0.7 : 0.35);

    // effect target label
    fade(this->getChildByID("item-menu")->getChildByTag(8),
        !opts["easy"], ANIM_TIME_L, !opts["easy"] ? 0.5 : 0.25, !opts["easy"] ? 0.5 : 0.25);

    // switch effect preview target in items menu
    m_effBundleCell->setModeTarget(this->gamemode);

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
    if (!(this->tab < 14 && !opts["easy"] || !this->tab))
        return;
    // dump settings
    this->dumpConfig();
    // get what to do
    int dir = sender->getTag() > 1 ? 1 : -1;

    // easy mode
    if (opts["easy"])
        this->channel = Channel((int(this->channel) + dir + 9) % 9);
    // in effects modify, switch gamemode
    else if (this->tab > 9) {
        this->gamemode = Gamemode((int(this->gamemode) + dir + 8) % 9 + 1);
        // refresh target of setup menu
        for (auto cell : CCArrayExt<SetupItemCell*>(m_setupAdvScroller->m_contentLayer->getChildren()))
            if (!cell->setModeTarget(this->gamemode))
                break;
    }
    // in icons modify, switch channel
    else if (this->channel == Channel::Main && dir == -1)
        this->channel = this->gamemode == Gamemode::Wave ? Channel::WaveTrail : (this->gamemode == Gamemode::Ufo ? Channel::UFOShell : Channel::TPLine);
    else if (this->channel == Channel::TPLine && dir == 1)
        this->channel = this->gamemode == Gamemode::Wave ? Channel::WaveTrail : (this->gamemode == Gamemode::Ufo ? Channel::UFOShell : Channel::Main);
    else if (this->channel == Channel::WaveTrail)
        this->channel = dir > 0 ? Channel::Main : Channel::TPLine;
    else if (this->channel == Channel::UFOShell)
        this->channel = dir > 0 ? Channel::Main : Channel::TPLine;
    else
        this->channel = Channel(int(this->channel) + dir);

    // load new setup
    currentSetup = setups[getIndex(this->ptwo, this->gamemode, this->channel)];

    // labels
    m_itemSetupLabel->setString(items[(int)this->gamemode].c_str());
    m_chnlSetupLabel->setString(chnls[(int)this->channel].c_str());

    // workspace
    this->m_workspace->runAction(CCSequence::create(
        CallFuncExt::create([this, dir] () {m_workspace->Fade(false, dir);}),
        CCDelayTime::create(dir ? ANIM_TIME_M / 3 : 0),
        CallFuncExt::create([this, dir] () {
            m_waitspace->refreshUI(currentSetup);
            m_waitspace->Fade(true, dir);
            auto temp = m_waitspace;
            m_waitspace = m_workspace;
            m_workspace = temp;            
        }),
        nullptr
    ));
}

void ChromaLayer::onOptionsPage(CCObject*) {
    auto cur = pages.back();
    pages.push_back(Page::Options);

    // fade out the old page
    fade(m_optionsBtn, false, ANIM_TIME_M);
    switch (cur) {
    case Page::Item:
        this->fadeItemPage();
        fade(m_modeBtn, false, ANIM_TIME_M);
        break;
    case Page::Setup:
        this->fadeSetupPage();
        fade(m_applyBtn, false, ANIM_TIME_M);
        break;
    case Page::Color:
        this->fadeColorPage();
        fade(m_applyBtn, false, ANIM_TIME_M);
        break;
    default:
        break;
    }
    this->runAction(CCSequence::create(
        CCDelayTime::create(ANIM_TIME_M),
        CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeOptionsPage)),
        CallFuncExt::create([this] () {
            fade(m_infoBtn, pages.back() == Page::Item || pages.back() == Page::Options, ANIM_TIME_M);
            fade(m_optionsBtn, pages.back() == Page::Item || pages.back() == Page::Setup || pages.back() == Page::Color, ANIM_TIME_M);
            fade(m_applyBtn, pages.back() == Page::Setup || pages.back() == Page::Color, ANIM_TIME_M);
        }),
        nullptr
    ));
}

void ChromaLayer::onInfoPage(CCObject*) {
    auto cur = pages.back();
    pages.push_back(Page::Info);

    // fade out old page
    fade(m_infoBtn, false, ANIM_TIME_M);
    switch (cur) {
    case Page::Item:
        this->fadeItemPage();
        fade(m_optionsBtn, false, ANIM_TIME_M);
        fade(m_modeBtn, false, ANIM_TIME_M);
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
            fade(m_infoBtn, pages.back() == Page::Item || pages.back() == Page::Options, ANIM_TIME_M);
            fade(m_optionsBtn, pages.back() == Page::Item || pages.back() == Page::Setup || pages.back() == Page::Color, ANIM_TIME_M);
            fade(m_applyBtn, pages.back() == Page::Setup || pages.back() == Page::Color, ANIM_TIME_M);
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
        this->clipColor.first = true;
        this->clipColor.second = oriColor;
        this->transistColorBtn(false, false);
        return;
    case 3:
        this->clipColor.first = true;
        this->clipColor.second = crtColor;
        this->transistColorBtn(true, false);
        return;
    case 5:
        this->clipSetup.first = true;
        this->clipSetup.second = currentSetup;
    }
}

void ChromaLayer::onPaste(CCObject* sender) {
    if (sender->getTag() == 6) {
        if (!clipSetup.first)
            return;
        currentSetup = this->clipSetup.second;
        this->dumpConfig();
        this->m_workspace->runAction(CCSequence::create(
            CallFuncExt::create([this] () {
                m_workspace->Fade(false);
                m_waitspace->refreshUI(currentSetup);
                auto temp = m_waitspace;
                m_waitspace = m_workspace;
                m_workspace = temp;
            }),
            CCDelayTime::create(ANIM_TIME_M / 3),
            CallFuncExt::create([this] () {
                m_workspace->Fade(true);
                }),
            nullptr
        ));
        return;
    }
    if (sender->getTag() != 2 || !clipColor.first)
        return;
    crtColor = this->clipColor.second;
    this->transistColorBtn(true, false);
    this->refreshColorPage(4);
    
}

void ChromaLayer::onResc(CCObject* sender) {
    crtColor = this->oriColor;
    this->transistColorBtn(false, false);
    this->refreshColorPage(4);
}

void ChromaLayer::onApply(CCObject* sender) {
    if (pages.back() == Page::Color) {
        // color target
        this->getColorTarget()->setColor(crtColor);
        // set setup value
        switch (colorTag) {
        case 1:
            currentSetup.color = crtColor;
            break;
        case 2:
            currentSetup.gradient.begin()->second = crtColor;
            break;
        case 3:
            currentSetup.gradient.rbegin()->second = crtColor;
            break;
        case 4:
            currentSetup.progress.begin()->second = crtColor;
            break;
        case 5:
            currentSetup.progress.rbegin()->second = crtColor;
            break;
        }
        this->dumpConfig();
    }
    // im speechlees of the delay fade design
    if (pages.back() != Page::Item)
        this->onClose(nullptr);
}

void ChromaLayer::onClose(CCObject* sender) {
    // mute on dragging slider
    if (this->on_slider || this->pages.back() == Page::Init)
        return;

    auto cur = pages.back();
    pages.pop_back();

    // fade out current page
    switch (cur) {
    case Page::Info:
        this->fadeInfoPage();
        break;
    case Page::Options:
        this->fadeOptionsPage();
        if (pages.back() != Page::Item)
            fade(m_infoBtn, false, ANIM_TIME_M);
        break;
    case Page::Color:
        this->fadeColorPage();
        break;
    case Page::Setup:
        this->fadeSetupPage();
        fade(m_applyBtn, false, ANIM_TIME_M);
        // dump config
        this->dumpConfig();
        break;
    // byebye menu
    case Page::Item:
        pages.pop_back();
        pages.push_back(Page::Terminal);
        this->fadeItemPage();
        pages.pop_back();
        this->fadeMainMenu();
        this->runAction(CCFadeTo::create(ANIM_TIME_L, 0));
        m_bg->runAction(CCFadeTo::create(ANIM_TIME_L, 0));
        // blur
        if (m_blur)
            m_blur->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_L, 0)));
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
    switch (pages.back()) {
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
            fade(m_infoBtn, pages.back() == Page::Item || pages.back() == Page::Options, ANIM_TIME_M);
            fade(m_optionsBtn, pages.back() == Page::Item || pages.back() == Page::Setup || pages.back() == Page::Color, ANIM_TIME_M);
            fade(m_applyBtn, pages.back() == Page::Setup || pages.back() == Page::Color, ANIM_TIME_M);
        }),
        nullptr
    ));
}