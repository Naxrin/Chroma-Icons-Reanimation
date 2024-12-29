// This file describes how the layer process direct callback from his buttons and signal from other menu classes inside.
#include "Layer.hpp"

extern std::map<short, ChromaSetup> setups;
extern std::map<std::string, bool> opts;
extern float speed;
extern bool ptwo;
extern std::map<PlayerObject*, bool> reset;
/**************** EVENT HANDLER *******************/
ListenerResult ChromaLayer::handleBoolSignal(SignalEvent<bool>* event) {
    // activate
    if (event->name == "activate") {
        opts["activate"] = event->value;
        if (!reset.empty())
            for (auto [key, _] : reset)
                reset[key] = !event->value;

        // item menu toggle preview
        m_advBundleCell->toggleChroma();
        m_ezyBundleCell->toggleChroma();
        m_effBundleCell->toggleChroma();

        // setup item
        for (auto obj : CCArrayExt<CCObject*>(m_setupAdvScroller->m_contentLayer->getChildren())) {
            auto cell = static_cast<SetupItemCell*>(obj);
            if (cell != m_currentItem)
                cell->m_btn->toggleChroma();
        }
        for (auto obj : CCArrayExt<CCObject*>(m_setupEasyScroller->m_contentLayer->getChildren())) {
            auto cell = static_cast<SetupItemCell*>(obj);
            if (cell != m_currentItem)
                cell->m_btn->toggleChroma();
        }
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
    else if (event->name == "best")
        currentSetup.best = event->value;

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
        this->refreshPreview(false);
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
        this->refreshPreview(false);
    }

    // set saturation
    else if (event->name == "satu") {
        this->currentSetup.satu = event->value;
        this->refreshPreview(false);
    }

    
    // pick an item from item page or setup page
    else if (event->name == "pick") {
        // from item menu icon
        if (this->face.back() == Face::Item) {
            this->switchCurrentID(event->value);
            this->m_workspace->refreshUI(currentSetup, false);
            this->face.push_back(Face::Setup);
            this->fadeItemPage();
            fade(m_infoBtn, false, ANIM_TIME_M);
            this->runAction(CCSequence::create(
                CCDelayTime::create(ANIM_TIME_M),
                CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeSetupPage)),
                CallFuncExt::create([this] () {
                    fade(m_infoBtn, face.back() == Face::Item || face.back() == Face::Options, ANIM_TIME_M);
                    fade(m_optionsBtn, face.back() == Face::Item || face.back() == Face::Setup || face.back() == Face::Color, ANIM_TIME_M);
                    fade(m_applyBtn, face.back() == Face::Setup || face.back() == Face::Color, ANIM_TIME_M);
                }),
                nullptr
            ));            
        }
        // from setup menu icon
        else if (this->face.back() == Face::Setup) {
            bool really_changed = this->switchCurrentID(event->value);
            if (really_changed) {
                // show or hide channel switch arrow
                fade(m_leftArrowSetupBtn, id < 10);
                fade(m_rightArrowSetupBtn, id < 10);
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
        auto cur = face.back();
        face.push_back(Face::Color);
        // maybe planning more
        switch (cur) {
        case Face::Setup:
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
                fade(m_infoBtn, face.back() == Face::Item || face.back() == Face::Options, ANIM_TIME_M);
                fade(m_optionsBtn, face.back() == Face::Item || face.back() == Face::Setup || face.back() == Face::Color, ANIM_TIME_M);
                fade(m_applyBtn, face.back() == Face::Setup || face.back() == Face::Color, ANIM_TIME_M);
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
            face.pop_back();
            this->fadeWarnPage();
            face.push_back(Face::Init);
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
    if (this->face.back() == Face::Setup)
        this->refreshPreview(true);
    // flip own p2 value
    ptwo = !ptwo;
    // flip his twin button
    if (sender->getTag() < 3)
        m_playerItemBtn->toggle(!m_playerSetupBtn->isToggled());
    else
        m_playerSetupBtn->toggle(!m_playerItemBtn->isToggled());

    // new config
    currentSetup = Mod::get()->getSavedValue<ChromaSetup>(getConfigKey(ptwo, this->id, this->channel), DEFAULT_SETUP);
    // if outside setup page, then refresh ui and return
    if (this->face.back() != Face::Setup) {
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
    // 1<->2 3<-->4
    opts["easy"] = !opts["easy"];
    // flip setting value
    Mod::get()->setSavedValue("easy-mode", opts["easy"]);
    // full mode
    m_advBundleCell->Fade(!opts["easy"]);
    // easy mode
    m_ezyBundleCell->Fade(opts["easy"]);

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
    // dump settings
    this->refreshPreview(true);
    // get what to do
    int dir = sender->getTag() > 1 ? 1 : -1;
    this->channel = Channel((int(channel) + dir + 3) % 3);
    this->m_chnlSetupLabel->setString(chnls[(int)channel].c_str());
    if (sender->getTag() == 3)
        dir = 0;
    // load new setup
    currentSetup = Mod::get()->getSavedValue<ChromaSetup>(getConfigKey(ptwo, this->id, this->channel), DEFAULT_SETUP);
    // crazy things
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
    auto cur = face.back();
    face.push_back(Face::Options);

    // fade out the old page
    fade(m_optionsBtn, false, ANIM_TIME_M);
    switch (cur) {
    case Face::Item:
        this->fadeItemPage();
        fade(m_modeBtn, false, ANIM_TIME_M);
        break;
    case Face::Setup:
        this->fadeSetupPage();
        fade(m_applyBtn, false, ANIM_TIME_M);
        break;
    case Face::Color:
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
            fade(m_infoBtn, face.back() == Face::Item || face.back() == Face::Options, ANIM_TIME_M);
            fade(m_optionsBtn, face.back() == Face::Item || face.back() == Face::Setup || face.back() == Face::Color, ANIM_TIME_M);
            fade(m_applyBtn, face.back() == Face::Setup || face.back() == Face::Color, ANIM_TIME_M);
        }),
        nullptr
    ));
}

void ChromaLayer::onInfo(CCObject*) {
    auto cur = face.back();
    face.push_back(Face::Info);

    // fade out old page
    fade(m_infoBtn, false, ANIM_TIME_M);
    switch (cur) {
    case Face::Item:
        this->fadeItemPage();
        fade(m_optionsBtn, false, ANIM_TIME_M);
        fade(m_modeBtn, false, ANIM_TIME_M);
        break;
    case Face::Options:
        this->fadeOptionsPage();
    default:
        break;
    }
    // delay enter info page
    this->runAction(CCSequence::create(
        CCDelayTime::create(ANIM_TIME_M),
        CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeInfoPage)),
        CallFuncExt::create([this] () {
            fade(m_infoBtn, face.back() == Face::Item || face.back() == Face::Options, ANIM_TIME_M);
            fade(m_optionsBtn, face.back() == Face::Item || face.back() == Face::Setup || face.back() == Face::Color, ANIM_TIME_M);
            fade(m_applyBtn, face.back() == Face::Setup || face.back() == Face::Color, ANIM_TIME_M);
        }),
        nullptr
    ));
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
        this->copied_color = true;
        this->clipColor = oriColor;
        this->transistColorBtn(false, false);
        return;
    case 3:
        this->copied_color = true;
        this->clipColor = crtColor;
        this->transistColorBtn(true, false);
        return;
    case 5:
        this->copied_setup = true;
        this->clipSetup = currentSetup;
    }
}

void ChromaLayer::onPaste(CCObject* sender) {
    if (sender->getTag() == 6) {
        if (!copied_setup)
            return;
        currentSetup = this->clipSetup;
        this->refreshPreview(false);
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
    if (sender->getTag() != 2 || !copied_color)
        return;
    crtColor = this->clipColor;
    this->transistColorBtn(true, false);
    this->refreshColorPage(4);
    
}

void ChromaLayer::onResc(CCObject* sender) {
    crtColor = this->oriColor;
    this->transistColorBtn(false, false);
    this->refreshColorPage(4);
}

void ChromaLayer::onApply(CCObject* sender) {
    if (face.back() == Face::Color) {
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
        this->refreshPreview(false);
    }
    // im speechlees of the delay fade design
    if (face.back() != Face::Item)
        this->onClose(nullptr);
}

void ChromaLayer::onClose(CCObject* sender) {
    // mute on dragging slider
    if (this->on_slider)
        return;

    auto cur = face.back();
    face.pop_back();

    // fade out current page
    switch (cur) {
    case Face::Info:
        this->fadeInfoPage();
        break;
    case Face::Options:
        this->fadeOptionsPage();
        if (face.back() != Face::Item)
            fade(m_infoBtn, false, ANIM_TIME_M);
        break;
    case Face::Color:
        this->fadeColorPage();
        break;
    case Face::Setup:
        this->fadeSetupPage();
        fade(m_applyBtn, false, ANIM_TIME_M);
        // dump config
        this->refreshPreview(true);
        break;
    // byebye menu
    case Face::Item:
        face.pop_back();
        face.push_back(Face::Terminal);
        this->fadeItemPage();
        face.pop_back();
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
    case Face::Warn:
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
    switch (face.back()) {
    case Face::Options:
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_M),
            CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeOptionsPage)),
            nullptr
        ));
        break;
    case Face::Color:
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_M),
            CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeColorPage)),
            nullptr
        ));
        break;
    case Face::Setup:
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_M),
            CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeSetupPage)),
            nullptr
        ));
        break;
    case Face::Item:
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
            fade(m_infoBtn, face.back() == Face::Item || face.back() == Face::Options, ANIM_TIME_M);
            fade(m_optionsBtn, face.back() == Face::Item || face.back() == Face::Setup || face.back() == Face::Color, ANIM_TIME_M);
            fade(m_applyBtn, face.back() == Face::Setup || face.back() == Face::Color, ANIM_TIME_M);
        }),
        nullptr
    ));
}