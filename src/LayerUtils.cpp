#include "Layer.hpp"

extern std::map<short, ChromaSetup> setups;
extern std::map<std::string, bool> opts;
extern float speed;

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
    this->m_phase = fmod(m_phase + 360 * d * speed, 360.f);
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
    // hint menu
    auto menuHint = CCMenu::create();
    menuHint->setContentSize(ccp(0.f, 0.f));
    menuHint->setScaleY(0.f);
    menuHint->setID("popup-menu");
    this->addChild(menuHint);

    auto mdContent = MDTextArea::create(content, ccp(333.f, height));
    mdContent->setScale(1.2);
    menuHint->addChild(mdContent);


    auto lbfTitle = CCLabelBMFont::create(title.c_str(), "ErasBold.fnt"_spr, 360.f);
    lbfTitle->setPositionY(0.6 * height + 18.f);
    hide(lbfTitle, 1, 0);
    lbfTitle->setID("info-title");
    lbfTitle->setTag(0);
    menuHint->addChild(lbfTitle);

    auto lbfOkay = CCLabelBMFont::create("Okay", "ErasBold.fnt"_spr, 200.f);
    lbfOkay->setScale(0.5);
    auto btnOkay = CCMenuItemSpriteExtra::create(lbfOkay, this, menu_selector(ChromaLayer::onClose));
    btnOkay->setPositionY(-0.6 * height - 15.f);
    hide(btnOkay, 1, 0);
    btnOkay->setID("info-button");
    btnOkay->setTag(1);
    menuHint->addChild(btnOkay);
}

// override ColorPickerDelegate function
void ChromaLayer::colorValueChanged(ccColor3B color) {
    this->m_crtColor = color;
    refreshColorPage(3);
}

