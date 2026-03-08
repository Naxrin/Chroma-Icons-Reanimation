#include "Layer.hpp"

extern std::map<short, ChromaPattern> setups;
extern std::map<std::string, bool> opts;
extern std::map<std::string, float> vals;
// reset chroma
std::map<PlayerObject*, bool> reset;

// process some init animation
void ChromaLayer::show() {
    // no i donot need it
    this->getChildByType<CCLayer>(0)->setVisible(false);
    // fade the bg from 0 to 120
    this->setOpacity(0);
    #ifdef GEODE_IS_WINDOWS
    GLubyte bgalpha = 170 - 70 * opts["blur-bg"];
    #else
    GLubyte bgalpha = 170;
    #endif
    this->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_L, bgalpha)));
    // check warn
    bool warn = false;
    if (Mod::get()->getSavedValue<bool>("notify", true))
        if (auto layer = PlayLayer::get()) {
            auto level = layer->m_level;
            log::error("{} {}", level->m_demon.value(), level->m_demonDifficulty);
            warn = level->m_demon.value() && level->m_demonDifficulty == 6;
        }

    if (warn) {
        // pages vector
        m_pages.push_back(Page::Warn);
        // fade in warn page
        this->fadeWarnPage();
    } else {
        // init page signal
        m_pages.push_back(Page::Init);
        // fade in main menu
        this->fadeMainMenu();
        // make item page
        this->makeItemPage();
        // fade in item page
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_M),
            CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeItemPage)),
            nullptr
        ));
    }

    // run base function
    Popup::show();
    //float opacity = 160.f;
    //m_bg->runAction(CCFadeTo::create(ANIM_TIME_L, opacity));
}

// schedule update rewrite
void ChromaLayer::update(float d) {
    // step phase
    this->m_phase = fmod(m_phase + 360 * d * vals["speed"], 360.f);
    this->m_percentage = fmod(m_percentage + 10 * d, 100.f);

    if (m_hasSetupPage && m_currentTab)
        // setup menu selected
        m_currentTab->runChroma(m_phase, m_percentage, 50);

    // skip
    if (!(opts["activate"] && opts["prev"]))
        return;

    // item menu
    m_cellItemAdv->runChroma(m_phase, m_percentage, 50);
    m_cellItemEasy->runChroma(m_phase, m_percentage, 50);
    m_cellItemEffect->runChroma(m_phase, m_percentage, 50);
}

CCMenuItemSpriteExtra* ChromaLayer::getColorTarget() {
    switch (m_colorTag) {
    case 1:
        m_lbfColorItem->setString("Static Color");
        return m_cellWorkspace->m_aryMenus[4]->m_colpk;
    case 2:
        m_lbfColorItem->setString("Gradient Color #1");
        return m_cellWorkspace->m_aryMenus[10]->m_colpk1;
    case 3:
        m_lbfColorItem->setString("Gradient Color #2");
        return m_cellWorkspace->m_aryMenus[10]->m_colpk2;
    case 4:
        m_lbfColorItem->setString("Progress Color @ 0%");
        return m_cellWorkspace->m_aryMenus[14]->m_colpk1;
    case 5:
        m_lbfColorItem->setString("Progress Color @ 100%");
        return m_cellWorkspace->m_aryMenus[14]->m_colpk2;
    }
    return nullptr;
}

void ChromaLayer::dumpConfig() {
    // dump setup
    Mod::get()->setSavedValue(getConfigKey(this->m_ptwo, this->m_gamemode, this->m_channel), m_currentSetup);
    // set value for in-game use
    setups[getIndex(this->m_ptwo, this->m_gamemode, this->m_channel)] = m_currentSetup;
}

bool ChromaLayer::switchTab(int tab) {
    // switch a nonsense
    if (this->m_tab == tab && m_pages.back() == Page::Setup)
        return false;

    // switch current tab
    if (m_currentTab) {
        // unselect
        m_currentTab->select(false);
        // dump config
        this->dumpConfig();
    }
    // to effect
    if (tab > 9) {
        this->m_channel = Channel(tab - 6);
        if (!opts["easy"]) {
            // force convert gamemode
            if (tab == 14)
                this->m_gamemode = Gamemode::Wave;
            else if (tab == 15)
                this->m_gamemode = Gamemode::Ufo;
        }
    }
    // to icon
    else {
        this->m_gamemode = Gamemode(tab);
        // from effect / from the two special channels
        if (this->m_tab > 9 || this->m_channel == Channel::WaveTrail || this->m_channel == Channel::UFOShell)
            this->m_channel = Channel::Main;
    }
    this->m_tab = tab;

    // load new setup
    m_currentSetup = setups[getIndex(this->m_ptwo, this->m_gamemode, this->m_channel)];

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

    // locate current tab
    m_currentTab = static_cast<SetupItemCell*>(
        (opts["easy"] ? m_scrollerSetupTabsEasy : m_scrollerSetupTabsAdv)->m_contentLayer->getChildByTag(tab ? 16 - tab : 7));
    // scroll the scroller to dest position
    // tint green
    if (m_currentTab)
        // select
        m_currentTab->select(true);
    else
        log::error("m_currentTab not found: tab = {}", tab);
    return true;
}

void ChromaLayer::makeHintPopup(std::string title, std::string content, float height) {
    if (!this->m_hasPopupPage) {
        this->makePopupPage();
        this->m_hasPopupPage = true;
    }

    if (auto md = m_menuHint->getChildByID("md"))
        md->removeFromParentAndCleanup(true);

    auto mdContent = MDTextArea::create(content, ccp(333.f, height));
    mdContent->setScale(1.2);
    //mdContent->getScrollLayer()->m_disableMovement = true;
    mdContent->getScrollLayer()->setTouchEnabled(false);
    mdContent->setID("md");
    log::debug("will add child");
    m_menuHint->addChild(mdContent);
    log::debug("child added");
    this->m_lbfHint->setString(title.c_str());
    this->m_lbfHint->setPositionY(0.6 * height + 18.f);

    this->m_btnOkay->setPositionY(-0.6 * height - 15.f);
}

// override ColorPickerDelegate function
void ChromaLayer::colorValueChanged(ccColor3B color) {
    this->m_crtColor = color;
    refreshColorPage(3);
}

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
        m_bg->setVisible(blur);
        this->setOpacity(170 - 70 * blur);
        return ListenerResult::Stop;
    }));

    this->m_radios.push_back(Signal<int>("blur-lvl").listen([this] (int level) -> ListenerResult {
        BlurAPI::getOptions(this->m_bg)->passes = level + 1;
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
                fade(m_btnSetupArrowLeft, showArrows, ANIM_TIME_L);
                fade(m_btnSetupArrowRight, showArrows, ANIM_TIME_L);

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
        vals["speed"] = speed;
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