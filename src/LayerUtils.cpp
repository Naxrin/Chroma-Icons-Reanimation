#include "Layer.hpp"


// schedule update rewrite
void ChromaLayer::update(float dt) {

}

std::string ChromaLayer::getConfigKey(bool space) {
    // get string
    std::string cat = space ? "  " : "-";
    std::string p = this->p2 ? "P2" : "P1";

    // concatenate
    if (this->id > 10)
        return p + cat + items[id-1];
    else
        return p + cat + items[id] + cat + chnls[(int)channel];
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

bool ChromaLayer::switchCurrentID(int id) {
    // avoid nullptr crash
    if (m_currentItem)
        // tint gray
        m_currentItem->m_label->runAction(CCEaseExponentialOut::create(
            CCTintTo::create(ANIM_TIME_M, 127, 127, 127)));
    
    bool changed = true;
    if (this->id != id) {
        if (m_currentItem)
            // dump former setup
            Mod::get()->setSavedValue(getConfigKey(), currentSetup);
        this->id = id;
        // labels
        m_itemSetupLabel->setString((id > 10 ? items[id-1] : items[id]).c_str());
        this->channel = id < 10 ? Channel::Main : Channel::Effect;
        m_chnlSetupLabel->setString(id < 10 ? "Main" : "Effect");

        // load new setup
        currentSetup = Mod::get()->getSavedValue<ChromaSetup>(getConfigKey(), DEFAULT_SETUP);
    } else
        changed = false;

    // locate new current setup item
    if (easy)
        m_currentItem = static_cast<SetupItemCell*>(m_setupEasyScroller->m_contentLayer->getChildByTag(id > 10 ? 16 - id : 6));
    else
        m_currentItem = static_cast<SetupItemCell*>(m_setupAdvScroller->m_contentLayer->getChildByTag(id > 10 ? 16 - id : 15 - id));
    // tint green
    if (m_currentItem)
        m_currentItem->m_label->runAction(CCEaseExponentialOut::create(
            CCTintTo::create(ANIM_TIME_M, 0, 255, 0)));
    else
        log::error("m_currentItem not found: id = {}", id);
    return changed;
}

// override ColorPickerDelegate function
void ChromaLayer::colorValueChanged(ccColor3B color) {
    this->crtColor = color;
    refreshColorPage(3);
}