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

    // toggle preview
    m_advBundleCell->toggleChroma();
    m_ezyBundleCell->toggleChroma();
    m_effBundleCell->toggleChroma();
    // setup item
    for (auto obj : CCArrayExt<CCObject*>(m_setupAdvScroller->m_contentLayer->getChildren())) {
        auto cell = static_cast<SetupItemCell*>(obj);
        if (cell != m_currentItem)
            cell->m_btn->toggleChroma(false);
    }
    for (auto obj : CCArrayExt<CCObject*>(m_setupEasyScroller->m_contentLayer->getChildren())) {
        auto cell = static_cast<SetupItemCell*>(obj);
        if (cell != m_currentItem)
            cell->m_btn->toggleChroma(false);
    }
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
        m_currentItem->m_btn->runChroma(phase, percentage, 50);

    // skip
    if (!(opts["activate"] && opts["prev"]))
        return;

    // item menu
    if (!m_advBundleCell->btns.empty())
        for (auto btn : m_advBundleCell->btns)
            btn->runChroma(phase, percentage, 50);
    if (!m_ezyBundleCell->btns.empty())
        for (auto btn : m_ezyBundleCell->btns)
            btn->runChroma(phase, percentage, 50);
    if (!m_effBundleCell->btns.empty())
        for (auto btn : m_effBundleCell->btns)
            btn->runChroma(phase, percentage, 50);
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
    Mod::get()->setSavedValue(getConfigKey(this->ptwo, this->id, (int)this->channel), currentSetup);
    setups[getIndex(this->ptwo, this->id, (int)this->channel)] = currentSetup;
}

bool ChromaLayer::switchCurrentID(int id) {
    // switch a nonsense
    if (this->id == id && pages.back() == Page::Setup)
        return false;

    if (m_currentItem) {
        // tint bg
        m_currentItem->switchTheme();
        // tint gray
        m_currentItem->m_label->runAction(CCEaseExponentialOut::create(
            CCTintTo::create(ANIM_TIME_M, 127, 127, 127)));
        // stop chroma
        m_currentItem->m_btn->toggleChroma(false);
        // dump config
        this->dumpConfig();
    }
    // change channel format
    int n = getIDType(id);
    // have to change channel
    if (getIDType(this->id) != n) {
        if (opts["easy"])
            this->channel = id ? Channel::Effect : Channel::Main;
        else
            this->channel = n == 2 ? Channel::Effect : (n == 1 ? Channel::Cube : Channel::Main);
    }
    // change target
    if (n == 1)
        this->target = this->channel;
    // edit id
    this->id = id;

    // labels
    m_itemSetupLabel->setString(items[id].c_str());
    m_chnlSetupLabel->setString(((int)this->channel < 5 ? chnls[(int)this->channel] : items[(int)this->channel - 4]).c_str());

    // load new setup
    currentSetup = setups[getIndex(this->ptwo, this->id, (int)this->channel)];

    // locate new current setup item
    int tag = id > 9 ? 16 - id : (opts["easy"] ? 6 : 15 - id);
    // loate current item
    m_currentItem = static_cast<SetupItemCell*>((opts["easy"] ? m_setupEasyScroller : m_setupAdvScroller)->m_contentLayer->getChildByTag(tag));
    // scroll the scroller to dest position
    // tint green
    if (m_currentItem) {
        // tint bg
        m_currentItem->tint(ANIM_TIME_M, 0, 80, 0);
        // tint gray
        m_currentItem->m_label->runAction(CCEaseExponentialOut::create(
            CCTintTo::create(ANIM_TIME_M, 0, 255, 0)));
        // start chroma
        m_currentItem->m_btn->toggleChroma(true);
    }
    else
        log::error("m_currentItem not found: id = {}", id);
    return true;
}

// override ColorPickerDelegate function
void ChromaLayer::colorValueChanged(ccColor3B color) {
    this->crtColor = color;
    refreshColorPage(3);
}

