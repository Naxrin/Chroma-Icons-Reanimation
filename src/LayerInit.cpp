#include "Layer.hpp"

#define HIDE(target, sX, sY) target->setScaleX(sX); target->setScaleY(sY); target->setVisible(false); target->setOpacity(0);
// makup initial UI
bool ChromaLayer::setup() {
    // set id
    this->setID("chroma-icons-central"_spr);
    // winSize
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    /********** Init Value **********/

    // bg regarding theme color
    this->m_bg = CCLayerColor::create(ccColor4B(BG_COLOR, 0));
    m_bg->setContentSize(winSize);
    m_bg->setZOrder(-2);
    m_bg->setID("bg");
    this->addChild(m_bg);

    // blur
    // for pre-release TheSillyDoggo's blur-bg is not needed
    // this mod make up blur background by himself
    if (auto blur = this->getChildByType<CCBlurLayer>(0))
        m_blur = blur;
    else {
        m_blur = CCBlurLayer::create();
        m_blur->setID("chroma-blur-layer");
        m_blur->setZOrder(-69);
        //m_blur->runAction(CCFadeTo::create(0.5f, 255));
        this->addChild(m_blur);
    }

    /********** Notify **********/
    this->m_warnPage = WarnCell::create();
    m_warnPage->setPosition(CCPoint(winSize.width/2, winSize.height/2));
    this->addChild(m_warnPage);
    m_cells.push_back(m_warnPage);
    
    /********** Title Label **********/
    // title label, will change
    auto titleLabel = TitleCell::create("Chroma Icons", CCPoint(winSize.width/2, winSize.height + 5.f), 200, 0, "title-label");
    titleLabel->setOpacity(0);
    titleLabel->setScale(0.5);
    this->addChild(titleLabel);
    m_cells.push_back(titleLabel);
    
    /********** Main Menu **********/
    auto mainMenu = CCMenu::create();
    mainMenu->setPosition(CCPoint(0, 0));
    mainMenu->setID("main-menu");
    this->addChild(mainMenu);

    // closeBtn
    auto exitSpr = CCSprite::create("closeBtn.png"_spr);
    exitSpr->setScale(0.8);
    exitSpr->setID("exit");
    this->m_exitBtn = CCMenuItemSpriteExtra::create(exitSpr, this, menu_selector(ChromaLayer::onClose));
    m_exitBtn->setPosition(CCPoint(30.f, winSize.height - 30.f));
    m_exitBtn->setColor(ccc3(CELL_COLOR));    
    m_exitBtn->setTag(1);
    mainMenu->addChild(m_exitBtn);

    // applyBtn
    auto applySpr = CCSprite::create("applyBtn.png"_spr);
    applySpr->setScale(0.8);
    applySpr->setID("apply");
    this->m_applyBtn = CCMenuItemSpriteExtra::create(applySpr, this, menu_selector(ChromaLayer::onApply));
    m_applyBtn->setPosition(CCPoint(winSize.width - 30.f, 30.f));
    m_applyBtn->setColor(ccc3(CELL_COLOR));
    m_applyBtn->setTag(5);
    HIDE(m_applyBtn, 0, 0)
    mainMenu->addChild(m_applyBtn);

    // infoBtn
    auto infoSpr = CCSprite::create("infoBtn.png"_spr);
    infoSpr->setScale(0.6);
    infoSpr->setID("info");
    this->m_infoBtn = CCMenuItemSpriteExtra::create(infoSpr, this, menu_selector(ChromaLayer::onInfo));    
    m_infoBtn->setPosition(CCPoint(winSize.width - 30.f, 30.f));
    m_infoBtn->setColor(ccc3(CELL_COLOR));    
    m_infoBtn->setTag(4);
    mainMenu->addChild(m_infoBtn);

    // optionsBtn
    auto gearSpr = CCSprite::create("optionsBtn.png"_spr);
    gearSpr->setScale(0.8);
    gearSpr->setID("option");
    this->m_optionsBtn = CCMenuItemSpriteExtra::create(gearSpr, this, menu_selector(ChromaLayer::onOptionsPage));
    m_optionsBtn->setPosition(CCPoint(winSize.width - 30.f, winSize.height - 30.f));
    m_optionsBtn->setTag(2);
    m_optionsBtn->setColor(ccc3(CELL_COLOR));    
    mainMenu->addChild(m_optionsBtn);

    // Easy-Adv switcher
    auto easySpr = CCSprite::create("icon_Easy.png"_spr);
    easySpr->setScale(0.9);
    easySpr->setID("easy");
    auto advSpr = CCSprite::create("icon_Adv.png"_spr);
    advSpr->setScale(0.9);
    advSpr->setID("adv");
    this->m_modeBtn = CCMenuItemToggler::create(advSpr, easySpr, this, menu_selector(ChromaLayer::onSwitchEasyAdv));
    m_modeBtn->setPosition(CCPoint(30.f, 30.f));
    m_modeBtn->setTag(3);
    m_modeBtn->setCascadeOpacityEnabled(true);
    m_modeBtn->setCascadeColorEnabled(true);
    m_modeBtn->setColor(ccc3(CELL_COLOR));    
    m_modeBtn->toggle(easy);
    mainMenu->addChild(m_modeBtn);

    mainMenu->setOpacity(0);
    mainMenu->setScale(1.4);

    /********** Item Menu **********/
    // item select menu
    auto itemMenu = CCMenu::create();
    itemMenu->setID("item-menu");
    this->addChild(itemMenu);

    // full items, easy item, effect row
    for (int wine = 1; wine < 4; wine ++) {
        auto itemBundle = ItemCell::create(wine);
        itemMenu->addChild(itemBundle);
        m_cells.push_back(itemBundle);
    }
    
    auto labelP1 = CCLabelBMFont::create("Player 1", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    labelP1->setScale(0.7);
    auto labelP2 = CCLabelBMFont::create("Player 2", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    labelP2->setScale(0.7);
    this->m_playerItemBtn = CCMenuItemToggler::create(labelP1, labelP2, this, menu_selector(ChromaLayer::onSwitchPlayer));
    m_playerItemBtn->setPosition(CCPoint(0, 100.f));
    m_playerItemBtn->setTag(4);
    m_playerItemBtn->setCascadeOpacityEnabled(true);
    m_playerItemBtn->setCascadeColorEnabled(true);
    HIDE(m_playerItemBtn, 5, 0.2)
    m_playerItemBtn->setEnabled(false);
    itemMenu->addChild(m_playerItemBtn);

    auto labelIcon = CCLabelBMFont::create("Player Icons", "ErasWhite.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    labelIcon->setPosition(CCPoint(0.f, 70.f));
    HIDE(labelIcon, 3.5, 0.14)
    labelIcon->setColor(ccc3(CELL_COLOR));
    labelIcon->setTag(5);
    itemMenu->addChild(labelIcon);

    auto labelEffect = CCLabelBMFont::create("In-Game Effects", "ErasWhite.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    labelEffect->setPosition(CCPoint(0.f, -25.f));
    HIDE(labelEffect, 3.5, 0.14)
    labelEffect->setColor(ccc3(CELL_COLOR));
    labelEffect->setTag(6);
    itemMenu->addChild(labelEffect);

    auto menuSpeed = SpeedSliderBundle::create(speed);
    itemMenu->addChild(menuSpeed);

    /********** Setup Menu **********/
    auto setupMenu = CCMenu::create();
    setupMenu->setID("setup-menu");
    this->addChild(setupMenu);
    
    auto labelP3 = CCLabelBMFont::create("Player 1", "ErasBold.fnt"_spr, 160.f, CCTextAlignment::kCCTextAlignmentLeft);
    labelP3->setScale(0.6);
    auto labelP4 = CCLabelBMFont::create("Player 2", "ErasBold.fnt"_spr, 160.f, CCTextAlignment::kCCTextAlignmentLeft);
    labelP4->setScale(0.6);
    this->m_playerSetupBtn = CCMenuItemToggler::create(labelP3, labelP4, this, menu_selector(ChromaLayer::onSwitchPlayer));
    m_playerSetupBtn->setPosition(CCPoint(85.f - winSize.width / 2, winSize.height / 2 - 25.f));
    m_playerSetupBtn->setTag(2);
    m_playerSetupBtn->setCascadeOpacityEnabled(true);
    m_playerSetupBtn->setCascadeColorEnabled(true);
    HIDE(m_playerSetupBtn, 1, 1)
    m_playerSetupBtn->setEnabled(false);
    setupMenu->addChild(m_playerSetupBtn);

    this->m_itemSetupLabel = CCLabelBMFont::create("Item", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentLeft);
    m_itemSetupLabel->setPosition(CCPoint(120.f - winSize.width / 2, winSize.height / 2 - 25.f));
    m_itemSetupLabel->setAnchorPoint(CCPoint(0.f, 0.5));
    HIDE(m_itemSetupLabel, 0.54, 0.54)
    m_itemSetupLabel->setID("current-item-label");
    setupMenu->addChild(m_itemSetupLabel);

    this->m_chnlSetupLabel = CCLabelBMFont::create("Channel", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentLeft);
    m_chnlSetupLabel->setPosition(CCPoint(60.f - winSize.width / 2, winSize.height / 2 - 45.f));
    m_chnlSetupLabel->setAnchorPoint(CCPoint(0.f, 0.5));
    m_chnlSetupLabel->setColor(ccc3(192, 192, 192));
    HIDE(m_chnlSetupLabel, 0.4, 0.4)
    m_chnlSetupLabel->setID("current-channel-label");
    setupMenu->addChild(m_chnlSetupLabel);

    this->m_setupEasyScroller = ScrollLayerPlus::create(CCRect(0.f, 0.f, 110.f, 300.f));
    m_setupEasyScroller->setAnchorPoint(CCPoint(0.5, 0.5));
	m_setupEasyScroller->setPosition(CCPoint(-160.f, 0.f));
    m_setupEasyScroller->ignoreAnchorPointForPosition(false);
	m_setupEasyScroller->setContentSize(CCSize(120.f, 260.f));
	m_setupEasyScroller->setID("easy-scroller");
    m_setupEasyScroller->setTag(6);
    m_setupEasyScroller->setVisible(false);
    m_setupEasyScroller->setCeiling();
    setupMenu->addChild(m_setupEasyScroller);

    this->m_setupAdvScroller = ScrollLayerPlus::create(CCRect(0.f, 0.f, 110.f, 620.f));
    m_setupAdvScroller->setAnchorPoint(CCPoint(0.5, 0.5));
	m_setupAdvScroller->setPosition(CCPoint(-160.f, 0.f));
    m_setupAdvScroller->ignoreAnchorPointForPosition(false);
	m_setupAdvScroller->setContentSize(CCSize(120.f, 260.f));
	m_setupAdvScroller->setID("advanced-scroller");
    m_setupAdvScroller->setTag(14);
    m_setupAdvScroller->setVisible(false);
    m_setupAdvScroller->setCeiling();
    setupMenu->addChild(m_setupAdvScroller);

    // add content
    float Y = 50.f;
    SetupItemCell* cell = nullptr;

    // effects
    for (int tag = 15; tag > 10; tag--) {
        cell = SetupItemCell::create(tag, Y, 16-tag);
        static_cast<MyContentLayer*>(m_setupEasyScroller->m_contentLayer)->addChild(cell);
        m_cells.push_back(cell);
        Y += 40.f;
    }
    // simple icon
    cell = SetupItemCell::create(0, Y, 6);
    static_cast<MyContentLayer*>(m_setupEasyScroller->m_contentLayer)->addChild(cell);
    m_cells.push_back(cell);
    Y = 50.f;

    // full icons
    for (int tag = 15; tag > 0; tag--) {
        // skip tag 10 cuz there is nonsense
        if (tag == 10)
            continue;
        cell = SetupItemCell::create(tag, Y, 15-tag + (tag > 10));
        static_cast<MyContentLayer*>(m_setupAdvScroller->m_contentLayer)->addChild(cell);
        m_cells.push_back(cell);
        Y += 40.f;
    }

    // workspace
    this->m_workspace = SetupOptionCell::create();
    setupMenu->addChild(m_workspace);
    m_cells.push_back(m_workspace);
    m_workspace->refreshUI(currentSetup);

    this->m_waitspace = SetupOptionCell::create();
    setupMenu->addChild(m_waitspace);
    m_cells.push_back(m_waitspace);
    
    auto arrowLeft = CCSprite::create("closeBtn.png"_spr);
    arrowLeft->setScale(0.6);
    arrowLeft->setID("channel-button-left");
    this->m_leftArrowSetupBtn = CCMenuItemSpriteExtra::create(arrowLeft, this, menu_selector(ChromaLayer::onSwitchChannelPage));
    m_leftArrowSetupBtn->setPosition(CCPoint(-90.f, 0.f));
    m_leftArrowSetupBtn->setColor(ccc3(CELL_COLOR));
    m_leftArrowSetupBtn->setTag(1);
    HIDE(m_leftArrowSetupBtn, 0, 0)
    m_leftArrowSetupBtn->setEnabled(false);
    setupMenu->addChild(m_leftArrowSetupBtn);

    auto arrowRight = CCSprite::create("closeBtn.png"_spr);
    arrowRight->setScale(0.6);
    arrowRight->setID("channel-button-right");
    arrowRight->setFlipX(true);
    this->m_rightArrowSetupBtn = CCMenuItemSpriteExtra::create(arrowRight, this, menu_selector(ChromaLayer::onSwitchChannelPage));
    m_rightArrowSetupBtn->setPosition(CCPoint(190.f, 0.f));
    m_rightArrowSetupBtn->setColor(ccc3(CELL_COLOR));    
    m_rightArrowSetupBtn->setTag(2);
    HIDE(m_rightArrowSetupBtn, 0, 0)
    m_rightArrowSetupBtn->setEnabled(false);
    setupMenu->addChild(m_rightArrowSetupBtn);

    // copyBtn
    auto copySpr = CCSprite::create("copyBtn.png"_spr);
    copySpr->setScale(0.8);
    copySpr->setID("copy");
    this->m_copyBtn = CCMenuItemSpriteExtra::create(copySpr, this, menu_selector(ChromaLayer::onCopy));
    m_copyBtn->setPosition(CCPoint(winSize.width / 2 - 30.f, winSize.height / 2 - 75.f));
    m_copyBtn->setTag(5);
    m_copyBtn->setColor(ccc3(CELL_COLOR));
    HIDE(m_copyBtn, 0, 0)
    setupMenu->addChild(m_copyBtn);

    // pasteBtn
    auto pasteSpr = CCSprite::create("pasteBtn.png"_spr);
    pasteSpr->setScale(0.8);
    pasteSpr->setID("paste");
    this->m_pasteBtn = CCMenuItemSpriteExtra::create(pasteSpr, this, menu_selector(ChromaLayer::onPaste));
    m_pasteBtn->setPosition(CCPoint(winSize.width / 2 - 30.f, winSize.height / 2 - 120.f));
    m_pasteBtn->setTag(6);
    m_pasteBtn->setColor(ccc3(CELL_COLOR));
    HIDE(m_pasteBtn, 0, 0)
    setupMenu->addChild(m_pasteBtn);

    /********** Color Menu **********/
    auto colorMenu = CCMenu::create();
    colorMenu->setID("color-menu");
    this->addChild(colorMenu);

    // title
    this->m_colorTitle = CCLabelBMFont::create("Pick a Color", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    m_colorTitle->setPosition(CCPoint(0.f, 130.f));
    HIDE(m_colorTitle, 0.35, 0.35)
    m_colorTitle->setTag(5);
    colorMenu->addChild(m_colorTitle);

    this->m_colorItem = CCLabelBMFont::create("target", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    m_colorItem->setPosition(CCPoint(0.f, 105.f));
    m_colorItem->setColor(ccc3(192, 192, 192));
    HIDE(m_colorItem, 0.2, 0.2)
    m_colorItem->setTag(6);
    colorMenu->addChild(m_colorItem);

    // pick color
    this->m_picker = CCControlColourPicker::colourPicker();
    m_picker->setPosition(CCPoint(-60.f, 0));
    m_picker->setContentSize(CCSize(0.f, 0.f));
    HIDE(m_picker, 0, 0)
    m_picker->setID("color-picker");
    colorMenu->addChild(m_picker);
    // red
    this->m_redCell = ColorValueCell::create(0);
    colorMenu->addChild(m_redCell);
    m_cells.push_back(m_redCell);
    // green
    this->m_greenCell = ColorValueCell::create(1);
    colorMenu->addChild(m_greenCell);
    m_cells.push_back(m_greenCell);
    // blue
    this->m_blueCell = ColorValueCell::create(2);
    colorMenu->addChild(m_blueCell);
    m_cells.push_back(m_blueCell);
    // hex
    this->m_hexCell = ColorHexCell::create();
    colorMenu->addChild(m_hexCell);
    m_cells.push_back(m_hexCell);

    auto oriColorSpr = ColorChannelSprite::create();
    this->m_oriColorDisplay = CCMenuItemSpriteExtra::create(oriColorSpr, this, menu_selector(ChromaLayer::onColorDisplayBtn));
    m_oriColorDisplay->setPosition(CCPoint(-180.f, 30.f));
    HIDE(m_oriColorDisplay, 0, 0)
    m_oriColorDisplay->setTag(1);
    colorMenu->addChild(m_oriColorDisplay);

    auto crtColorSpr = ColorChannelSprite::create();
    this->m_crtColorDisplay = CCMenuItemSpriteExtra::create(crtColorSpr, this, menu_selector(ChromaLayer::onColorDisplayBtn));
    m_crtColorDisplay->setPosition(CCPoint(-180.f, -30.f));
    HIDE(m_crtColorDisplay, 0, 0)
    m_crtColorDisplay->setTag(3);
    colorMenu->addChild(m_crtColorDisplay);

    // arrow
    this->m_mysteriousArrow = CCSprite::create("mysteriousArrow.png"_spr);
    m_mysteriousArrow->setPosition(CCPoint(-180.f, 0.f));
    HIDE(m_mysteriousArrow, 0.3, 0.3)
    m_mysteriousArrow->setColor(ccc3(CELL_COLOR));
    colorMenu->addChild(m_mysteriousArrow);

    // copy Ori
    auto copyOriSpr = CCSprite::create("copyBtn.png"_spr);
    copyOriSpr->setScale(0.8);
    copyOriSpr->setID("copy");
    this->m_copyOriBtn = CCMenuItemSpriteExtra::create(copyOriSpr, this, menu_selector(ChromaLayer::onCopy));
    m_copyOriBtn->setPosition(CCPoint(-180.f, 30.f));
    HIDE(m_copyOriBtn, 0, 0)
    m_copyOriBtn->setTag(1);
    m_copyOriBtn->setColor(ccc3(CELL_COLOR));
    colorMenu->addChild(m_copyOriBtn);

    // resc Ori
    auto rescOriSpr = CCSprite::create("rescBtn.png"_spr);
    rescOriSpr->setScale(0.75);
    rescOriSpr->setID("resc");
    this->m_rescOriBtn = CCMenuItemSpriteExtra::create(rescOriSpr, this, menu_selector(ChromaLayer::onResc));
    m_rescOriBtn->setPosition(CCPoint(-180.f, 30.f));
    m_rescOriBtn->setTag(4);
    HIDE(m_rescOriBtn, 0, 0)
    m_rescOriBtn->setColor(ccc3(CELL_COLOR));
    colorMenu->addChild(m_rescOriBtn);

    // copy Crt
    auto copyCrtSpr = CCSprite::create("copyBtn.png"_spr);
    copyCrtSpr->setScale(0.8);
    copyCrtSpr->setID("copy");
    this->m_copyCrtBtn = CCMenuItemSpriteExtra::create(copyCrtSpr, this, menu_selector(ChromaLayer::onCopy));
    m_copyCrtBtn->setPosition(CCPoint(-180.f, -30.f));
    m_copyCrtBtn->setTag(3);
    HIDE(m_copyCrtBtn, 0, 0)
    m_copyCrtBtn->setColor(ccc3(CELL_COLOR));
    colorMenu->addChild(m_copyCrtBtn);

    // paste Crt
    auto pasteCrtSpr = CCSprite::create("pasteBtn.png"_spr);
    pasteCrtSpr->setScale(0.8);
    pasteCrtSpr->setID("paste");
    this->m_pasteCrtBtn = CCMenuItemSpriteExtra::create(pasteCrtSpr, this, menu_selector(ChromaLayer::onPaste));
    m_pasteCrtBtn->setPosition(CCPoint(-180.f, -30.f));
    m_pasteCrtBtn->setTag(2);
    HIDE(m_pasteCrtBtn, 0, 0)
    m_pasteCrtBtn->setColor(ccc3(CELL_COLOR));
    colorMenu->addChild(m_pasteCrtBtn);

    /********** Options Menu **********/
    auto optionsMenu = CCMenu::create();
    optionsMenu->setID("options-menu");
    this->addChild(optionsMenu);

    // optionsScroller
    // fullscreen scroll tolerance
    this->m_optionScroller = ScrollLayerPlus::create(CCRect(0.f, 0.f, 320.f, 400.f));
    m_optionScroller->setAnchorPoint(CCPoint(0.5, 0.5));
    m_optionScroller->ignoreAnchorPointForPosition(false);
	m_optionScroller->setContentSize(winSize);//CCSize(winSize.width, winSize.height)
    m_optionScroller->m_contentLayer->setPositionX(winSize.width / 2 - 160.f);
	m_optionScroller->setID("options-scroller");
    m_optionScroller->setVisible(false);
    optionsMenu->addChild(m_optionScroller);

    float H = 50;
    int sTag = 0;

    sTag ++;
    auto darkOpt = OptionTogglerCell::create("Dark Theme", H, 300, true, sTag, "dark-theme",
        "Deep Dark Fantasy...");
    static_cast<MyContentLayer*>(m_optionScroller->m_contentLayer)->addChild(darkOpt);
    H += darkOpt->getContentHeight() + 15.f;

    sTag ++;
    auto blurOpt = OptionTogglerCell::create("Blur Background", H, 300, true, sTag, "blur-bg",
        "Add a gaussian blur effect to the background. Needs Blur BG (by TheSillyDoggo) to be loaded");
    static_cast<MyContentLayer*>(m_optionScroller->m_contentLayer)->addChild(blurOpt);
    H += blurOpt->getContentHeight() + 15.f;

    sTag ++;
    auto prevOpt = OptionTogglerCell::create("Preview Effects", H, 300, true, sTag, "prev-effects",
        "Preview Chroma Effects in menu.");
    static_cast<MyContentLayer*>(m_optionScroller->m_contentLayer)->addChild(prevOpt);
    H += prevOpt->getContentHeight() + 15.f;

    sTag ++;
    auto uiTitle = OptionTitleCell::create("Interface Options", H, 300, sTag, "interface-title");
    static_cast<MyContentLayer*>(m_optionScroller->m_contentLayer)->addChild(uiTitle);
    H += 40.f;

    sTag ++;
    auto sepglowOpt = OptionTogglerCell::create("Seperate Glow Color Phase", H, 300, false, sTag, "sep-glow",
        "Set phase of Glow Color keeps 120 degrees delay from Main Color.\nOtherwise Glow Color aligns with Main Color");
    static_cast<MyContentLayer*>(m_optionScroller->m_contentLayer)->addChild(sepglowOpt);
    H += sepglowOpt->getContentHeight() + 15.f;

    sTag ++;
    auto sepsecondOpt = OptionTogglerCell::create("Seperate Secondary Color Phase", H, 300, false, sTag, "sep-second",
        "Set phase of Secondary Color keeps 120 degrees lead from Main Color.\nOtherwise Secondary Color aligns with Main Color");
    static_cast<MyContentLayer*>(m_optionScroller->m_contentLayer)->addChild(sepsecondOpt);
    H += sepsecondOpt->getContentHeight() + 15.f;

    sTag ++;
    auto sepdualOpt = OptionTogglerCell::create("Seperate Dual Mode Phase", H, 300, false, sTag, "sep-dual",
        "Set color phase of P2 keeps 180 degrees away from P1.\n"
        "Otherwise the two players use the same phase are their color are the same everytime.");
    static_cast<MyContentLayer*>(m_optionScroller->m_contentLayer)->addChild(sepdualOpt);
    H += sepdualOpt->getContentHeight() + 15.f;

    sTag ++;
    auto phaseTitle = OptionTitleCell::create("Phase Options", H, 300, sTag, "phase-title");
    static_cast<MyContentLayer*>(m_optionScroller->m_contentLayer)->addChild(phaseTitle);
    H += 40.f;

    sTag ++;
    auto editorOpt = OptionTogglerCell::create("Editor Test", H, 300, false, sTag, "editor",
        "Apply to Editor Playtest. Will also add a button in your editor menu if activated.\n But I do not promise your device will not lag.");
    static_cast<MyContentLayer*>(m_optionScroller->m_contentLayer)->addChild(editorOpt);
    H += editorOpt->getContentHeight() + 15.f;

    sTag ++;
    auto riderOpt = OptionTogglerCell::create("Seperate Riders", H, 300, false, sTag, "rider",
        "Set the cube rider of ship/ufo/jetpack will follow Cube mode's color.\nOtherwise she follow her vehicle's color.");
    static_cast<MyContentLayer*>(m_optionScroller->m_contentLayer)->addChild(riderOpt);
    H += riderOpt->getContentHeight() + 15.f;

    sTag ++;
    auto samedualOpt = OptionTogglerCell::create("Same Dual Color Mode", H, 300, false, sTag, "same-dual",
        "Merge Player 2's color setup with Player 1.\nNot fully equal to general Same Dual Color things,"
        "but you can also uncheck Seperate Dual Mode Phase if u hope that.");
    static_cast<MyContentLayer*>(m_optionScroller->m_contentLayer)->addChild(samedualOpt);
    H += samedualOpt->getContentHeight() + 15.f;

    sTag ++;
    auto generalTitle = OptionTitleCell::create("General Options", H, 300, sTag, "general-title");
    static_cast<MyContentLayer*>(m_optionScroller->m_contentLayer)->addChild(generalTitle);
    H += 40.f;

    sTag ++;
    auto switchOpt = OptionTogglerCell::create("Switch", H, 300, true, sTag, "activate",
        "Ultimate Switch of this mod.\nYou aren't required to reboot GD to toggle ON/OFF this mod!");
    static_cast<MyContentLayer*>(m_optionScroller->m_contentLayer)->addChild(switchOpt);
    H += switchOpt->getContentHeight() + 15.f;

    m_optionScroller->m_contentLayer->setContentHeight(H - 10.f);
    m_optionScroller->setTag(sTag);
    m_optionScroller->setCeiling();
    
    /********** Info Menu **********/
    auto infoMenu = CCMenu::create();
    infoMenu->setID("info-menu");
    this->addChild(infoMenu);

    auto lazy = CCLabelBMFont::create(
        "As I mentioned in Github,\nthis pre-release does NOTHING to ur icons.\n"
        "I'm too lazy to make this info page in this pre-release.\n"        
        "See you in official release!\n@_@",
        "ErasBold.fnt"_spr, 460.f, CCTextAlignment::kCCTextAlignmentCenter);
    lazy->setID("lazy-label");
    HIDE(lazy, 0.25, 0.25)
    infoMenu->addChild(lazy);
    // update
    //this->scheduleUpdate();
    return true;
}