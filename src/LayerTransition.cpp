#include "Layer.hpp"

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
        face.push_back(Face::Warn);
        this->fadeWarnPage();
    } else {
        face.push_back(Face::Init);
        fade(m_applyBtn, false, 0);
        fade(m_copyBtn, false, 0);
        fade(m_pasteBtn, false, 0);
        
        this->fadeMainMenu();
        this->runAction(CCSequence::create(
            CCDelayTime::create(0.6 * ANIM_TIME_L),
            CCCallFunc::create(this, callfunc_selector(ChromaLayer::fadeItemPage)),
            nullptr
        ));
    }
    // run base function
    Popup::show();
    float opacity = 196.f;
    // blur
    // for pre-release blur-bg is not needed
    //if (Loader::get()->isModLoaded("thesillydoggo.blur_bg"))
    if (auto node = this->getChildByType<CCBlurLayer>(0)) {
        // not my self-made background mask
        if (node->getID() == "bg")
            return;
        m_blur = node;
        if (Mod::get()->getSavedValue<bool>("blur-bg", true)) {
            node->runAction(CCEaseExponentialOut::create(CCFadeIn::create(ANIM_TIME_L)));   
            opacity = 144.f;
        }

        else {
            node->setVisible(false);
            node->setOpacity(0);
        }
    }
    m_bg->runAction(CCFadeTo::create(ANIM_TIME_L, opacity));
}

void ChromaLayer::updateSpeedValue(float value) {
    this->speed = value;
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
        obj->m_bg->runAction(CCTintTo::create(ANIM_TIME_M, CELL_COLOR));

    // item labels
    static_cast<CCLabelBMFont*>(this->getChildByID("item-menu")->getChildByTag(5))->setColor(ccColor3B(CELL_COLOR));
    static_cast<CCLabelBMFont*>(this->getChildByID("item-menu")->getChildByTag(6))->setColor(ccColor3B(CELL_COLOR));

    // setup channel switch arrows
    m_leftArrowSetupBtn->setColor(ccColor3B(CELL_COLOR));
    m_rightArrowSetupBtn->setColor(ccColor3B(CELL_COLOR));

    // setup copy paste
    m_copyBtn->setColor(ccColor3B(CELL_COLOR));
    m_pasteBtn->setColor(ccColor3B(CELL_COLOR));

    // color pick menu buttons
    m_copyOriBtn->setColor(ccColor3B(CELL_COLOR));
    m_rescOriBtn->setColor(ccColor3B(CELL_COLOR));
    m_copyCrtBtn->setColor(ccColor3B(CELL_COLOR));
    m_pasteCrtBtn->setColor(ccColor3B(CELL_COLOR));

    // color pick menu arrow
    m_mysteriousArrow->setColor(ccColor3B(CELL_COLOR));

    // options scroller cells
    int TAG = m_optionScroller->getTag();
    int tag = 1;
    while (tag <= TAG) {
        if (auto opt = m_optionScroller->m_contentLayer->getChildByTag(tag)) {
            opt->runAction(CCSequence::create(
                CCDelayTime::create(0.03*tag-0.03),
                CallFuncExt::create([opt]() {
                    static_cast<BaseCell*>(opt)->m_bg->runAction(CCTintTo::create(ANIM_TIME_M, CELL_COLOR));
                }),
                nullptr
            ));
            tag ++;
        } else
            break;
    }
}

/*
void ChromaLayer::fadeMainMenuButtons() {
    bool in1 = face.back() == Face::Init || face.back() == Face::Item;
    bool in2 = face.back() == Face::Setup || face.back() == Face::Color;
    // options
    fade(m_optionsBtn, in1 || in2, ANIM_TIME_M, in1 || in2 ? 1 : 0.5, in1 || in2 ? 1 : 0.5);
    // easy adv
    fade(m_modeBtn, in1, ANIM_TIME_M, in1 ? 1 : 0.5, in1 ? 1 : 0.5);
    // copy paste apply
    fade(m_copyBtn, in2, ANIM_TIME_M, in2 ? 1 : 0.5, in2 ? 1 : 0.5);
    fade(m_pasteBtn, in2, ANIM_TIME_M, in2 ? 1 : 0.5, in2 ? 1 : 0.5);
    fade(m_applyBtn, in2, ANIM_TIME_M, in2 ? 1 : 0.5, in2 ? 1 : 0.5);
    // info
    bool in3 = in1 || face.back() == Face::Options;
    fade(m_infoBtn, in3, ANIM_TIME_M, in3 ? 1 : 0.5, in3 ? 1 : 0.5);
}*/

void ChromaLayer::refreshColorPage(int type) {
    // set color title
    if (type != 4)
    m_colorTitle->setString(getConfigKey(true).c_str());

    // edit displayers
    m_crtColorDisplay->setColor(crtColor);    
    if (!type) {
        this->oriColor = crtColor;
        m_oriColorDisplay->setColor(crtColor);
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
    fade(boy, display, 0.5 * ANIM_TIME_M, display);
    boy->runAction(CCEaseExponentialOut::create(CCMoveTo::create(
        0.5 * ANIM_TIME_M, display ? CCPoint(-230.f, k*40.f) : CCPoint(-180.f, k*30.f))));

    fade(girl, display, 0.5 * ANIM_TIME_M, display);
    girl->runAction(CCEaseExponentialOut::create(CCMoveTo::create(
        0.5 * ANIM_TIME_M, display ? CCPoint(-190.f, k*80.f) : CCPoint(-180.f, k*30.f))));
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
    // title label
    static_cast<TitleCell*>(this->getChildByID("title-label"))->Fade(in);
    // item menu
    static_cast<ItemCell*>(this->getChildByID("item-menu")->getChildByTag(easy ? 2 : 1))->Fade(in);
    static_cast<ItemCell*>(this->getChildByID("item-menu")->getChildByTag(3))->Fade(in);
    // labels
    fade(m_playerItemBtn, in, ANIM_TIME_L, in ? 1 : 5, in ? 1 : 0.2);
    fade(this->getChildByID("item-menu")->getChildByTag(5), in, ANIM_TIME_L, in ? 0.7 : 3.5, in ? 0.7 : 0.14);
    fade(this->getChildByID("item-menu")->getChildByTag(6), in, ANIM_TIME_L, in ? 0.7 : 3.5, in ? 0.7 : 0.14);
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
    if (this->easy)
        m_setupEasyScroller->Transition(in);
    else
        m_setupAdvScroller->Transition(in);

    // player btn
    fade(m_playerSetupBtn, in, ANIM_TIME_L, 0.8, 0.8);
    fade(m_itemSetupLabel, in, ANIM_TIME_L, 0.48, 0.48);

    fade(m_chnlSetupLabel, in, ANIM_TIME_L, 0.32, 0.32);

    fade(m_leftArrowSetupBtn, in);
    fade(m_rightArrowSetupBtn, in);

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
    m_optionScroller->Transition(in);
}

void ChromaLayer::fadeInfoPage() {
    bool in = face.back() == Face::Info;

    fade(this->getChildByID("info-menu")->getChildByID("lazy-label"),
        in, ANIM_TIME_L, in ? 0.5 : 0.25, in ? 0.5 : 0.25);
}