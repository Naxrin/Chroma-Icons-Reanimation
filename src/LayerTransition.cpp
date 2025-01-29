#include "Layer.hpp"

extern std::map<short, ChromaSetup> setups;
extern std::map<std::string, bool> opts;
extern float speed;

void ChromaLayer::switchTheme() {
    // bg
    m_bg->runAction(CCTintTo::create(ANIM_TIME_M, BG_COLOR));

    // main-menu buttons 
    for (int tag = 1; tag < 6; tag ++)
        if (auto btn = getChildByID("main-menu")->getChildByTag(tag))
            //if (tag != 3)
            btn->runAction(CCTintTo::create(ANIM_TIME_M, CELL_COLOR));

    if (this->m_hasOptionsPage) {
        // options scroller cells
        int TAG = this->m_scrollerOptions->getTag();
        int tag = 1;
        while (tag <= TAG) {
            if (auto opt = this->m_scrollerOptions->m_contentLayer->getChildByTag(tag)) {
                opt->runAction(CCSequence::create(
                    CCDelayTime::create(0.03*tag-0.03),
                    CallFuncExt::create([this, opt, tag, TAG] () {
                        if (tag == TAG)
                            static_cast<BaseCell*>(opt)->tint(ANIM_TIME_M, 80 * (!opts["activate"]), 80 * opts["activate"], 0);
                        else
                            static_cast<BaseCell*>(opt)->switchTheme();
                    }),
                    nullptr
                ));
                tag ++;
            } else
                break;
        }        
    }

    
    // items page
    for (auto obj : CCArrayExt<BaseCell*>(m_cells))
        obj->switchTheme();

    // click this player icon to modify
    static_cast<CCLabelBMFont*>(this->getChildByID("item-menu")->getChildByTag(5))->setColor(ccc3(CELL_COLOR));
    // advanced mode player icons label
    static_cast<CCLabelBMFont*>(this->getChildByID("item-menu")->getChildByTag(6))->setColor(ccc3(CELL_COLOR));
    // in-game effects label
    static_cast<CCLabelBMFont*>(this->getChildByID("item-menu")->getChildByTag(7))->setColor(ccc3(CELL_COLOR));
    // preview target gamemode indicator
    static_cast<CCLabelBMFont*>(this->getChildByID("item-menu")->getChildByTag(8))->setColor(ccc3(CELL_COLOR));

    if (this->m_hasSetupPage) {
        // setup channel switch arrows
        m_btnSetupArrowLeft->setColor(ccc3(CELL_COLOR));
        m_btnSetupArrowRight->setColor(ccc3(CELL_COLOR));

        // setup c+p
        m_btnSetupCopy->setColor(ccc3(CELL_COLOR));
        m_btnSetupPaste->setColor(ccc3(CELL_COLOR));        
    }

    if (this->m_hasColorPage) {
        // color pick menu buttons
        m_btnColorCopyOri->setColor(ccc3(CELL_COLOR));
        m_btnColorRescOri->setColor(ccc3(CELL_COLOR));
        m_btnColorCopyCur->setColor(ccc3(CELL_COLOR));
        m_btnColorPasteCur->setColor(ccc3(CELL_COLOR));

        // color pick menu arrow
        m_sprArrow->setColor(ccc3(CELL_COLOR));
    }

    if (this->m_hasInfoPage) {
        // info menu
        for (auto btn : CCArrayExt<CCMenuItemSpriteExtra*>(this->getChildByID("info-menu")->getChildByID("manual-menu")->getChildren()))
            btn->setColor(ccc3(CELL_COLOR));
        for (auto btn : CCArrayExt<CCMenuItemSpriteExtra*>(this->getChildByID("info-menu")->getChildByID("author-menu")->getChildren()))
            btn->setColor(ccc3(CELL_COLOR));
    }
}

void ChromaLayer::refreshColorPage(int type) {
    // edit displayers
    m_btnColorDisplayCur->setColor(m_crtColor);
    if (!type) {
        this->m_oriColor = m_crtColor;
        m_btnColorDisplayOri->setColor(m_crtColor);
        // set color title
        m_lbfColorTarget->setString(
            std::regex_replace(getConfigKey(this->m_ptwo, this->m_gamemode, this->m_channel),
                std::regex("-"), "  ").c_str());
    }

    // not from single rgb change
    if (type != 1) {
        m_cellRed->setVal(m_crtColor.r);
        m_cellGreen->setVal(m_crtColor.g);
        m_cellBlue->setVal(m_crtColor.b);
    }
    // not from hex text
    if (type != 2)
        m_cellHex->setColorValue(m_crtColor);

    // not from the picker panel
    if (type != 3) {
        // edit picker
        m_picker->setDelegate(nullptr);
        m_picker->setColorValue(m_crtColor);
        m_picker->setDelegate(this);
    }
}

void ChromaLayer::transistColorBtn(bool isCrt, bool display) {
    CCMenuItemSpriteExtra* parent = isCrt ? m_btnColorDisplayCur : m_btnColorDisplayCur;
    CCMenuItemSpriteExtra* boy = isCrt ? m_btnColorCopyCur : m_btnColorCopyOri;
    CCMenuItemSpriteExtra* girl = isCrt ? m_btnColorPasteCur : m_btnColorRescOri;

    parent->setTag(1 + isCrt * 2 + display);
    int k = isCrt ? -1 : 1;

    fade(boy, display, 0.5 * ANIM_TIME_M);
    boy->runAction(CCEaseExponentialOut::create(CCMoveTo::create(
        0.5 * ANIM_TIME_M, display ? CCPoint(-205.f, k*80.f) : CCPoint(-180.f, k*30.f))));

    fade(girl, display, 0.5 * ANIM_TIME_M);
    girl->runAction(CCEaseExponentialOut::create(CCMoveTo::create(
        0.5 * ANIM_TIME_M, display ? CCPoint(-155.f, k*80.f) : CCPoint(-180.f, k*30.f))));
}

// fade main menu
void ChromaLayer::fadeMainMenu() {
    bool in = !m_pages.empty();
    
    this->getChildByID("main-menu")->runAction(CCSpawn::create(
        CCFadeTo::create(ANIM_TIME_L, in * 255),
        CCEaseExponentialOut::create(CCScaleTo::create(ANIM_TIME_L, 1.4 - 0.4 * in)),
        nullptr
    ));
}

void ChromaLayer::fadeWarnPage() {
    bool in = !m_pages.empty();
    if (in) {
        // construct warning page
        this->m_cellWarning = WarnCell::create();
        m_cellWarning->setPosition(CCPoint(m_winSize.width/2, m_winSize.height/2));
        m_cellWarning->setID("warning");
        this->addChild(m_cellWarning);
        m_cellWarning->Fade(true);
    } else {
        m_cellWarning->Fade(false);
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_L),
            CallFuncExt::create([this] () { m_cellWarning->removeFromParentAndCleanup(true); }),
            nullptr
        ));
    }
}

void ChromaLayer::fadeItemPage() {
    // case Item : return from another Page
    // case terminal : init / exit
    bool in = m_pages.back() == Page::Item || m_pages.back() == Page::Init;
    // title labelthis->getChildByID("item-menu")->getChildByID("title-label")
    fade(m_lbfTitle, in, ANIM_TIME_M, in ? 0.6 : 0.3, in ? 0.6 : 0.3);
    // prepare effects
    if (in && !opts["easy"]) {
        static_cast<CCLabelBMFont*>(this->getChildByID("item-menu")->getChildByTag(8))
            ->setString(fmt::format("- {} -", items[(int)this->m_gamemode]).c_str());
        // refresh item menu target
        m_cellItemEffect->setModeTarget(this->m_gamemode);
    }
    // item menu
    (opts["easy"] ? m_cellItemEasy : m_cellItemAdv)->Fade(in);
    m_cellItemEffect->Fade(in);
    // labels
    fade(m_btnItemPlayer, in, ANIM_TIME_L, in ? 1 : 5, in ? 1 : 0.2);
    fade(this->getChildByID("item-menu")->getChildByTag(opts["easy"] ? 5 : 6), in, ANIM_TIME_L, in ? 0.7 : 0.35, in ? 0.7 : 0.35);
    fade(this->getChildByID("item-menu")->getChildByTag(7), in, ANIM_TIME_L, in ? 0.7 : 0.35, in ? 0.7 : 0.35);
    if (!opts["easy"])
        fade(this->getChildByID("item-menu")->getChildByTag(8), in, ANIM_TIME_L, in ? 0.5 : 0.25, in ? 0.5 : 0.25);
    // easy adv
    fade(m_btnMode, in, ANIM_TIME_M);
    // speed menu
    static_cast<SpeedSliderBundle*>(this->getChildByID("item-menu")->getChildByID("speed-menu"))->Fade(in);
    // init addition
    if (m_pages.back() == Page::Init)
        m_pages.push_back(Page::Item);
}

void ChromaLayer::fadeSetupPage() {
    bool in = m_pages.back() == Page::Setup;
    // switch between easy and adv
    if (opts["easy"])
        m_scrollerSetupTabsEasy->Transition(in, m_tab > 9 ? 16 - m_tab : 7);
    else
        m_scrollerSetupTabsAdv->Transition(in, 16 - m_tab);

    // player btn
    fade(m_btnSetupPlayer, in, ANIM_TIME_L);
    fade(m_btnSetupGamemode, in, ANIM_TIME_L);

    fade(m_btnSetupChannel, in, ANIM_TIME_L);

    // display channel switch arrows if necessary
    bool showArrows = this->m_tab < 14 && !opts["easy"] || !this->m_tab;
    fade(m_btnSetupArrowLeft, in && showArrows);
    fade(m_btnSetupArrowRight, in && showArrows);

    fade(m_btnSetupCopy, in, ANIM_TIME_M);
    fade(m_btnSetupPaste, in, ANIM_TIME_M);

    m_cellWorkspace->Fade(in);
}

void ChromaLayer::fadeColorPage() {
    bool in = m_pages.back() == Page::Color;

    // color menu title
    fade(m_lbfColorTarget, in, ANIM_TIME_L, in ? 0.7 : 0.35, in ? 0.7 : 0.35);
    fade(m_lbfColorItem, in, ANIM_TIME_L, in ? 0.4 : 0.2, in ? 0.4 : 0.2);

    // color displayer
    fade(m_btnColorDisplayCur, in, ANIM_TIME_L);
    fade(m_btnColorDisplayOri, in, ANIM_TIME_L);

    // buttons
    if (m_btnColorDisplayOri->getTag() == 2)
        this->transistColorBtn(false, false);
    if (m_btnColorDisplayCur->getTag() == 4)
    this->transistColorBtn(true, false);

    // mysterious arrow
    fade(m_sprArrow, in, ANIM_TIME_L, in ? 0.6 : 0.3, in ? 0.6 : 0.3);

    // cells
    m_cellRed->Fade(in);
    m_cellGreen->Fade(in);
    m_cellBlue->Fade(in);
    m_cellHex->Fade(in);

    // picker
    m_picker->setDelegate(in ? this : nullptr);
    fade(m_picker, in, ANIM_TIME_L, in ? 1 : 0, in ? 1 : 0);

    // i give in to cascading their opacity, forgive me with this plz
    for (int i = 0; i < 7; i++)
        m_picker->getChildByType<CCSpriteBatchNode>(0)->getChildByType<CCSprite>(i)->runAction(
            CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_L, 255*in)));
}

void ChromaLayer::fadeOptionsPage() {
    bool in = m_pages.back() == Page::Options;
    // animation
    // move offset not solved, give false for now
    m_scrollerOptions->Transition(in, 0);
}

void ChromaLayer::fadePopupPage() {
    bool in = m_pages.back() == Page::Popup;
    auto menu = this->getChildByID("popup-menu");
    if (!in && !menu)
        return;
    for (auto node : CCArrayExt<CCNode*>(menu->getChildren())) {
        if (node->getTag())
            fade(static_cast<CCMenuItem*>(node), in, ANIM_TIME_M);
        else {
            auto id = node->getID();
            float scale = stof(id);
            fade(node, in, ANIM_TIME_M, in ? scale : scale / 2, in ? scale : scale / 2);
        }

    }
    if (!in)
        this->runAction(CCSequence::create(
            CCDelayTime::create(ANIM_TIME_M),
            CallFuncExt::create([menu] () { menu->removeFromParentAndCleanup(true); }),
            nullptr
        ));
}

void ChromaLayer::fadeInfoPage() {
    bool in = m_pages.back() == Page::Info;

    fade(this->getChildByID("info-menu")->getChildByID("about"),
        in, ANIM_TIME_L, in ? 0.64 : 0.32, in ? 0.64 : 0.32);
    fade(this->getChildByID("info-menu")->getChildByID("version"),
        in, ANIM_TIME_L, in ? 0.3 : 0.15, in ? 0.3 : 0.15);
    fade(this->getChildByID("info-menu")->getChildByID("manual"),
        in, ANIM_TIME_L, in ? 0.5 : 0.2, in ? 0.5 : 0.25);
    fade(this->getChildByID("info-menu")->getChildByID("author"),
        in, ANIM_TIME_L, in ? 0.5 : 0.2, in ? 0.5 : 0.25);

    // btns
    for (auto obj: CCArrayExt<CCNode*>(this->getChildByID("info-menu")->getChildByID("manual-menu")->getChildren()))
        fade(obj, in);
    for (auto obj: CCArrayExt<CCNode*>(this->getChildByID("info-menu")->getChildByID("author-menu")->getChildren()))
        fade(obj, in);

    // thanks
    fade(this->getChildByID("info-menu")->getChildByID("thanks-title"),
        in, ANIM_TIME_L, in ? 0.5 : 0.25, in ? 0.5 : 0.25);
    fade(this->getChildByID("info-menu")->getChildByID("thanks-content"),
        in, ANIM_TIME_L, in ? 0.32 : 0.16, in ? 0.32 : 0.16);
}