#include "Layer.hpp"

extern std::map<short, ChromaSetup> setups;
extern std::map<std::string, bool> opts;
extern float speed;
extern bool ptwo;

void ChromaLayer::updateSpeedValue(float value) {
    speed = value;
}

void ChromaLayer::switchTheme() {
    // bg
    m_bg->runAction(CCTintTo::create(ANIM_TIME_M, BG_COLOR));

    // main-menu buttons 
    for (int tag = 1; tag < 6; tag ++)
        if (auto btn = getChildByID("main-menu")->getChildByTag(tag))
            btn->runAction(CCTintTo::create(ANIM_TIME_M, CELL_COLOR));

    // items page
    for (auto obj : CCArrayExt<BaseCell*>(m_cells))
        obj->switchTheme();

    // item labels
    static_cast<CCLabelBMFont*>(this->getChildByID("item-menu")->getChildByTag(5))->setColor(ccc3(CELL_COLOR));
    static_cast<CCLabelBMFont*>(this->getChildByID("item-menu")->getChildByTag(6))->setColor(ccc3(CELL_COLOR));
    static_cast<CCLabelBMFont*>(this->getChildByID("item-menu")->getChildByTag(7))->setColor(ccc3(CELL_COLOR));
    // setup channel switch arrows
    m_leftArrowSetupBtn->setColor(ccc3(CELL_COLOR));
    m_rightArrowSetupBtn->setColor(ccc3(CELL_COLOR));

    // setup copy paste
    m_copyBtn->setColor(ccc3(CELL_COLOR));
    m_pasteBtn->setColor(ccc3(CELL_COLOR));

    // color pick menu buttons
    m_copyOriBtn->setColor(ccc3(CELL_COLOR));
    m_rescOriBtn->setColor(ccc3(CELL_COLOR));
    m_copyCrtBtn->setColor(ccc3(CELL_COLOR));
    m_pasteCrtBtn->setColor(ccc3(CELL_COLOR));

    // color pick menu arrow
    m_mysteriousArrow->setColor(ccc3(CELL_COLOR));

    // options scroller cells
    int TAG = m_optionScroller->getTag();
    int tag = 1;
    while (tag <= TAG) {
        if (auto opt = m_optionScroller->m_contentLayer->getChildByTag(tag)) {
            opt->runAction(CCSequence::create(
                CCDelayTime::create(0.03*tag-0.03),
                CallFuncExt::create([this, opt, tag, TAG]() {
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

void ChromaLayer::refreshColorPage(int type) {
    // edit displayers
    m_crtColorDisplay->setColor(crtColor);    
    if (!type) {
        this->oriColor = crtColor;
        m_oriColorDisplay->setColor(crtColor);
        // set color title
        m_colorTitle->setString(
            std::regex_replace(getConfigKey(ptwo, this->id, (int)this->channel),
                std::regex("-"), "  ").c_str());
    }

    // not from single rgb change
    if (type != 1) {
        m_redCell->setVal(crtColor.r);
        m_greenCell->setVal(crtColor.g);
        m_blueCell->setVal(crtColor.b);
    }
    // not from hex text
    if (type != 2)
        m_hexCell->setColorValue(crtColor);

    // not from the picker panel
    if (type != 3) {
        // edit picker
        m_picker->setDelegate(nullptr);
        m_picker->setColorValue(crtColor);
        m_picker->setDelegate(this);
    }
}

void ChromaLayer::transistColorBtn(bool isCrt, bool display) {
    CCMenuItemSpriteExtra* parent = isCrt ? m_crtColorDisplay : m_oriColorDisplay;
    CCMenuItemSpriteExtra* boy = isCrt ? m_copyCrtBtn : m_copyOriBtn;
    CCMenuItemSpriteExtra* girl = isCrt ? m_pasteCrtBtn : m_rescOriBtn;

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
    bool in = !face.empty();
    
    this->getChildByID("main-menu")->runAction(CCSpawn::create(
        CCFadeTo::create(ANIM_TIME_L, in * 255),
        CCEaseExponentialOut::create(CCScaleTo::create(ANIM_TIME_L, 1.4 - 0.4 * in)),
        nullptr
    ));
}

void ChromaLayer::fadeWarnPage() {
    bool in = !face.empty();
    m_warnPage->Fade(in);
}

void ChromaLayer::fadeItemPage() {
    // case Item : return from another face
    // case terminal : init / exit
    bool in = face.back() == Face::Item || face.back() == Face::Init;
    // title labelthis->getChildByID("item-menu")->getChildByID("title-label")
    fade(m_titleLabel, in, ANIM_TIME_M, in ? 0.6 : 0.3, in ? 0.6 : 0.3);
    // prepare effects
    if (in) {
        static_cast<CCLabelBMFont*>(this->getChildByID("item-menu")->getChildByTag(7))
            ->setString(("- " + items[(int)this->target - 4] + " -").c_str());
        for (auto btn : m_effBundleCell->btns)
            btn->setModeTarget(this->target);
    }
    // item menu
    (opts["easy"] ? m_ezyBundleCell : m_advBundleCell)->Fade(in);
    m_effBundleCell->Fade(in);
    // labels
    fade(m_playerItemBtn, in, ANIM_TIME_L, in ? 1 : 5, in ? 1 : 0.2);
    fade(this->getChildByID("item-menu")->getChildByTag(5), in, ANIM_TIME_L, in ? 0.7 : 0.35, in ? 0.7 : 0.35);
    fade(this->getChildByID("item-menu")->getChildByTag(6), in, ANIM_TIME_L, in ? 0.7 : 0.35, in ? 0.7 : 0.35);
    if (!opts["easy"])
        fade(this->getChildByID("item-menu")->getChildByTag(7), in, ANIM_TIME_L, in ? 0.5 : 0.25, in ? 0.5 : 0.25);
    // easy adv
    fade(m_modeBtn, in, ANIM_TIME_M);
    // speed menu
    static_cast<SpeedSliderBundle*>(this->getChildByID("item-menu")->getChildByID("speed-menu"))->Fade(in);
    // init addition
    if (face.back() == Face::Init)
        face.push_back(Face::Item);

}

void ChromaLayer::fadeSetupPage() {
    bool in = face.back() == Face::Setup;
    // switch between easy and adv
    if (opts["easy"])
        m_setupEasyScroller->Transition(in, id > 10 ? 16 - id : 6);
    else
        m_setupAdvScroller->Transition(in, id > 10 ? 16 - id : 15 - id);

    // player btn
    fade(m_playerSetupBtn, in, ANIM_TIME_L, 0.8, 0.8);
    fade(m_itemSetupLabel, in, ANIM_TIME_L, 0.48, 0.48);

    fade(m_chnlSetupLabel, in, ANIM_TIME_L, 0.32, 0.32);

    if (id != 12 && id != 15) {
        fade(m_leftArrowSetupBtn, in);
        fade(m_rightArrowSetupBtn, in);
    }

    fade(m_copyBtn, in, ANIM_TIME_M);
    fade(m_pasteBtn, in, ANIM_TIME_M);
    m_workspace->Fade(in);
}
void ChromaLayer::fadeColorPage() {
    bool in = face.back() == Face::Color;

    // color menu title
    fade(m_colorTitle, in, ANIM_TIME_L, in ? 0.7 : 0.35, in ? 0.7 : 0.35);
    fade(m_colorItem, in, ANIM_TIME_L, in ? 0.4 : 0.2, in ? 0.4 : 0.2);

    // color displayer
    fade(m_oriColorDisplay, in, ANIM_TIME_L, in, in);
    fade(m_crtColorDisplay, in, ANIM_TIME_L,  in, in);

    // buttons
    if (m_oriColorDisplay->getTag() == 2)
        this->transistColorBtn(false, false);
    if (m_crtColorDisplay->getTag() == 4)
    this->transistColorBtn(true, false);

    // mysterious arrow
    fade(m_mysteriousArrow, in, ANIM_TIME_L, in ? 0.6 : 0.3, in ? 0.6 : 0.3);

    // cells
    m_redCell->Fade(in);
    m_greenCell->Fade(in);
    m_blueCell->Fade(in);
    m_hexCell->Fade(in);

    // picker
    m_picker->setDelegate(in ? this : nullptr);
    //m_picker->setZOrder(in ? 0 : -1);
    fade(m_picker, in, ANIM_TIME_L, in ? 1 : 0, in ? 1 : 0);

    // i give in to cascading their opacity, forgive me with this plz
    for (int i=0; i<7; i++)
        m_picker->getChildByType<CCSpriteBatchNode>(0)->getChildByType<CCSprite>(i)->runAction(
            CCEaseExponentialOut::create(CCFadeTo::create(ANIM_TIME_L, 255*in)));
}

void ChromaLayer::fadeOptionsPage() {
    bool in = face.back() == Face::Options;
    // animation
    // move offset not solved, give false for now
    m_optionScroller->Transition(in, 0);
}

void ChromaLayer::fadeInfoPage() {
    bool in = face.back() == Face::Info;

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
        in, ANIM_TIME_L, in ? 0.5 : 0.2, in ? 0.5 : 0.25);        
    fade(this->getChildByID("info-menu")->getChildByID("thanks-content"),
        in, ANIM_TIME_L, in ? 0.4 : 0.2, in ? 0.4 : 0.2);
}