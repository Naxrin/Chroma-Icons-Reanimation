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
    this->runAction(CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_L, 100)));
    // check warn
    bool warn = Mod::get()->getSavedValue<bool>("notify", true) && PlayLayer::get()
        && PlayLayer::get()->m_level->m_demonDifficulty == 6;
    if (warn) {
        pages.push_back(Page::Warn);
        this->fadeWarnPage();
    } else {
        pages.push_back(Page::Init);
        
        this->fadeMainMenu();
        this->runAction(CCSequence::create(
            CCDelayTime::create(0.6 * ANIM_TIME_L),
            CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeItemPage)),
            nullptr
        ));
    }

    for (auto cell : CCArrayExt<SetupItemCell*>(m_setupAdvScroller->m_contentLayer->getChildren()))
        if (!cell->setModeTarget(this->gamemode))
            break;

    // toggle preview
    m_advBundleCell->toggleChroma();
    m_ezyBundleCell->toggleChroma();
    m_effBundleCell->toggleChroma();
    // setup items
    for (auto cell : CCArrayExt<SetupItemCell*>(m_setupAdvScroller->m_contentLayer->getChildren()))
        cell->toggleChroma(cell == m_currentItem);
    for (auto cell : CCArrayExt<SetupItemCell*>(m_setupEasyScroller->m_contentLayer->getChildren()))
        cell->toggleChroma(cell == m_currentItem);
    // run base function
    Popup::show();
    float opacity = 196.f;
    // blur
        if (Mod::get()->getSavedValue<bool>("blur-bg", true)) {
            m_blur->runAction(CCEaseExponentialOut::create(CCFadeIn::create(ANIM_TIME_L)));   
            opacity = 144.f;
        }
        else {
            m_blur->setVisible(false);
            m_blur->setOpacity(0);
        }

    m_bg->runAction(CCFadeTo::create(ANIM_TIME_L, opacity));
}

// schedule update rewrite
void ChromaLayer::update(float d) {
    // step phase
    this->phase = fmod(phase + 360 * d * speed, 360.f);
    this->percentage = fmod(percentage + 10 * d, 100.f);
    if (m_currentItem)
        // setup menu selected
        m_currentItem->runChroma(phase, percentage, 50);

    // skip
    if (!(opts["activate"] && opts["prev"]))
        return;

    // item menu
    m_advBundleCell->runChroma(phase, percentage, 50);
    m_ezyBundleCell->runChroma(phase, percentage, 50);
    m_effBundleCell->runChroma(phase, percentage, 50);
}

CCMenuItemSpriteExtra* ChromaLayer::getColorTarget() {
    switch (colorTag) {
    case 1:
        m_colorItem->setString("Static Color");
        return m_workspace->m_aryMenus[4]->m_colpk;
    case 2:
        m_colorItem->setString("Gradient Color #1");
        return m_workspace->m_aryMenus[10]->m_colpk1;
    case 3:
        m_colorItem->setString("Gradient Color #2");
        return m_workspace->m_aryMenus[10]->m_colpk2;
    case 4:
        m_colorItem->setString("Progress Color @ 0%");
        return m_workspace->m_aryMenus[14]->m_colpk1;
    case 5:
        m_colorItem->setString("Progress Color @ 100%");
        return m_workspace->m_aryMenus[14]->m_colpk2;
    }
    return nullptr;
}

void ChromaLayer::dumpConfig() {
    // dump setup
    Mod::get()->setSavedValue(getConfigKey(this->ptwo, this->gamemode, this->channel), currentSetup);
    setups[getIndex(this->ptwo, this->gamemode, this->channel)] = currentSetup;
}

bool ChromaLayer::switchTab(int tab) {
    // switch a nonsense
    if (this->tab == tab && pages.back() == Page::Setup)
        return false;

    if (m_currentItem) {
        // unselect
        m_currentItem->select(false);
        // dump config
        this->dumpConfig();
    }
    // to effect
    if (tab > 9) {
        this->channel = Channel(tab - 6);
        if (!opts["easy"]) {
            // force convert gamemode
            if (tab == 14)
                this->gamemode = Gamemode::Wave;
            else if (tab == 15)
                this->gamemode = Gamemode::Ufo;
        }
    }
    // to icon
    else {
        this->gamemode = Gamemode(tab);
        // from effect / from the two special channels
        if (this->tab > 9 || this->channel == Channel::WaveTrail || this->channel == Channel::UFOShell)
            this->channel = Channel::Main;
    }
    this->tab = tab;

    // load new setup
    currentSetup = setups[getIndex(this->ptwo, this->gamemode, this->channel)];

    // labels
    m_itemSetupLabel->setString(items[(int)this->gamemode].c_str());
    m_chnlSetupLabel->setString(chnls[(int)this->channel].c_str());

    // loate current item
    m_currentItem = static_cast<SetupItemCell*>(
        (opts["easy"] ? m_setupEasyScroller : m_setupAdvScroller)->m_contentLayer->getChildByTag(tab ? 16 - tab : 7));
    // scroll the scroller to dest position
    // tint green
    if (m_currentItem)
        // select
        m_currentItem->select(true);
    else
        log::error("m_currentItem not found: tab = {}", tab);
    return true;
}

// override ColorPickerDelegate function
void ChromaLayer::colorValueChanged(ccColor3B color) {
    this->crtColor = color;
    refreshColorPage(3);
}

