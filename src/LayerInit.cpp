#include "Layer.hpp"

extern std::map<std::string, bool> opts;
extern float speed;

// makup initial UI (only main page)
bool ChromaLayer::setup() {
    // set id
    this->setID("chroma-icons-central"_spr);
    this->m_gamemode = this->m_gamemodeAdv = opts["easy"] ? Gamemode::Icon : Gamemode::Cube;

    /********** BG **********/

    // bg regarding theme color
    this->m_bg = CCLayerColor::create(ccc4(BG_COLOR, 0));
    m_bg->setContentSize(m_winSize);
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
    
    /********** Main Menu **********/
    auto menuMain = CCMenu::create();
    menuMain->setPosition(CCPoint(0, 0));
    menuMain->setID("main-menu");
    this->addChild(menuMain);

    // close button
    auto sprExit = CCSprite::create("closeBtn.png"_spr);
    sprExit->setScale(0.8);
    sprExit->setID("exit");
    this->m_btnExit = CCMenuItemSpriteExtra::create(sprExit, this, menu_selector(ChromaLayer::onClose));
    m_btnExit->setPosition(CCPoint(30.f, m_winSize.height - 30.f));
    m_btnExit->setColor(ccc3(CELL_COLOR));    
    m_btnExit->setTag(1);
    menuMain->addChild(m_btnExit);

    // apply button
    auto sprApply = CCSprite::create("applyBtn.png"_spr);
    sprApply->setScale(0.8);
    sprApply->setID("apply");
    this->m_btnApply = CCMenuItemSpriteExtra::create(sprApply, this, menu_selector(ChromaLayer::onApply));
    m_btnApply->setPosition(CCPoint(m_winSize.width - 30.f, 30.f));
    m_btnApply->setColor(ccc3(CELL_COLOR));
    m_btnApply->setTag(5);
    hide(m_btnApply, 0);
    menuMain->addChild(m_btnApply);

    // info button
    auto sprInfo = CCSprite::create("infoBtn.png"_spr);
    sprInfo->setScale(0.6);
    sprInfo->setID("info");
    this->m_btnInfo = CCMenuItemSpriteExtra::create(sprInfo, this, menu_selector(ChromaLayer::onInfoPage));    
    m_btnInfo->setPosition(CCPoint(m_winSize.width - 30.f, 30.f));
    m_btnInfo->setColor(ccc3(CELL_COLOR));
    m_btnInfo->setTag(4);
    menuMain->addChild(m_btnInfo);

    // options button
    auto sprGear = CCSprite::create("optionsBtn.png"_spr);
    sprGear->setScale(0.8);
    sprGear->setID("option");
    this->m_btnOptions = CCMenuItemSpriteExtra::create(sprGear, this, menu_selector(ChromaLayer::onOptionsPage));
    m_btnOptions->setPosition(CCPoint(m_winSize.width - 30.f, m_winSize.height - 30.f));
    m_btnOptions->setTag(2);
    m_btnOptions->setColor(ccc3(CELL_COLOR));
    menuMain->addChild(m_btnOptions);

    // Easy-Adv switcher
    auto sprEasy = CCSprite::create("icon_Easy.png"_spr);
    sprEasy->setScale(0.9);
    sprEasy->setID("easy");
    auto sprAdv = CCSprite::create("icon_Adv.png"_spr);
    sprAdv->setScale(0.9);
    sprAdv->setID("adv");
    this->m_btnMode = CCMenuItemToggler::create(sprAdv, sprEasy, this, menu_selector(ChromaLayer::onSwitchEasyAdv));
    m_btnMode->setPosition(CCPoint(30.f, 30.f));
    m_btnMode->setTag(3);
    m_btnMode->setCascadeOpacityEnabled(true);
    m_btnMode->setCascadeColorEnabled(true);
    //m_modeBtn->setColor(opts["easy"] ? ccc3(127, 127, 255) : ccc3(255, 127, 127));
    m_btnMode->setColor(ccc3(CELL_COLOR));    
    m_btnMode->toggle(opts["easy"]);
    menuMain->addChild(m_btnMode);

    menuMain->setOpacity(0);
    menuMain->setScale(1.4);

    return true;
}

void ChromaLayer::makeItemPage() {
    // item menu
    auto menuItem = CCMenu::create();
    menuItem->setID("item-menu");
    this->addChild(menuItem);

    this->m_lbfTitle = CCLabelBMFont::create("Chroma Icons Central", "ErasBold.fnt"_spr, 500.f, CCTextAlignment::kCCTextAlignmentCenter);
    m_lbfTitle->setPosition(CCPoint(0, m_winSize.height / 2 - 30.f));
    m_lbfTitle->setWidth(500.f);
    hide(m_lbfTitle, 0.3);
    m_lbfTitle->setID("title-label");
    menuItem->addChild(m_lbfTitle);

    // full items, easy item, effect row
    this->m_cellItemAdv = ItemCell::create(1);
    menuItem->addChild(m_cellItemAdv);
    m_cells.push_back(m_cellItemAdv);

    // full items, easy item, effect row
    this->m_cellItemEasy = ItemCell::create(2);
    menuItem->addChild(m_cellItemEasy);
    m_cells.push_back(m_cellItemEasy);

    // full items, easy item, effect row
    this->m_cellItemEffect = ItemCell::create(3);
    menuItem->addChild(m_cellItemEffect);
    m_cells.push_back(m_cellItemEffect);

    auto lbfP1 = CCLabelBMFont::create("Player 1", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    lbfP1->setScale(0.7);
    auto lbfP2 = CCLabelBMFont::create("Player 2", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    lbfP2->setScale(0.7);
    this->m_btnItemPlayer = CCMenuItemToggler::create(lbfP1, lbfP2, this, menu_selector(ChromaLayer::onSwitchPlayer));
    m_btnItemPlayer->setPosition(CCPoint(0, 95.f));
    m_btnItemPlayer->setTag(4);
    m_btnItemPlayer->setCascadeOpacityEnabled(true);
    m_btnItemPlayer->setCascadeColorEnabled(true);
    hide(m_btnItemPlayer, 5, 0.2);
    m_btnItemPlayer->setEnabled(false);
    menuItem->addChild(m_btnItemPlayer);

    auto lbfIconEasy = CCLabelBMFont::create("Click this Player Icon to Start", "ErasWhite.fnt"_spr, 360.f, CCTextAlignment::kCCTextAlignmentCenter);
    lbfIconEasy->setPosition(CCPoint(0.f, 60.f));
    hide(lbfIconEasy, 0.35);
    lbfIconEasy->setColor(ccc3(CELL_COLOR));
    lbfIconEasy->setTag(5);
    menuItem->addChild(lbfIconEasy);

    auto lbfIconsAdv = CCLabelBMFont::create("Player Icons", "ErasWhite.fnt"_spr, 360.f, CCTextAlignment::kCCTextAlignmentCenter);
    lbfIconsAdv->setPosition(CCPoint(0.f, 55.f));
    hide(lbfIconsAdv, 0.35);
    lbfIconsAdv->setColor(ccc3(CELL_COLOR));
    lbfIconsAdv->setTag(6);
    menuItem->addChild(lbfIconsAdv);

    auto lbfEffect = CCLabelBMFont::create("In-Game Effects", "ErasWhite.fnt"_spr, 360.f, CCTextAlignment::kCCTextAlignmentCenter);
    lbfEffect->setPosition(CCPoint(0.f, -35.f));
    hide(lbfEffect, 0.35);
    lbfEffect->setColor(ccc3(CELL_COLOR));
    lbfEffect->setTag(7);
    menuItem->addChild(lbfEffect);

    auto lbfTarget = CCLabelBMFont::create("- Cube -", "ErasWhite.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    lbfTarget->setPosition(CCPoint(0.f, -90.f));
    hide(lbfTarget, 0.25);
    lbfTarget->setColor(ccc3(CELL_COLOR));
    lbfTarget->setTag(8);
    menuItem->addChild(lbfTarget);

    // speed menu
    auto menuSpeed = SpeedSliderBundle::create(speed);
    menuItem->addChild(menuSpeed);

    // toggle preview
    m_cellItemEasy->toggleChroma();
    m_cellItemAdv->toggleChroma();
    m_cellItemEffect->toggleChroma();

    this->m_hasItemPage = true;

    // update
    this->scheduleUpdate();
}

void ChromaLayer::makeSetupPage() {
    // setup menu
    auto menuSetup = CCMenu::create();
    menuSetup->setID("setup-menu");
    this->addChild(menuSetup);
    
    auto lbfP1 = CCLabelBMFont::create("Player 1", "ErasBold.fnt"_spr, 160.f, CCTextAlignment::kCCTextAlignmentCenter);
    lbfP1->setScale(0.48);
    auto lbfP2 = CCLabelBMFont::create("Player 2", "ErasBold.fnt"_spr, 160.f, CCTextAlignment::kCCTextAlignmentCenter);
    lbfP2->setScale(0.48);
    this->m_btnSetupPlayer = CCMenuItemToggler::create(lbfP1, lbfP2, this, menu_selector(ChromaLayer::onSwitchPlayer));
    m_btnSetupPlayer->setPosition(CCPoint(85.f - m_winSize.width / 2, m_winSize.height / 2 - 25.f));
    m_btnSetupPlayer->setTag(2);
    m_btnSetupPlayer->setCascadeOpacityEnabled(true);
    m_btnSetupPlayer->setCascadeColorEnabled(true);
    hide(m_btnSetupPlayer, 0);
    m_btnSetupPlayer->setEnabled(false);
    m_btnSetupPlayer->toggle(this->m_ptwo);
    menuSetup->addChild(m_btnSetupPlayer);

    auto txtSetupGamemode = CCLabelBMFont::create("Gamemode", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    txtSetupGamemode->setScale(0.48);
    txtSetupGamemode->setID("label");
    this->m_btnSetupGamemode = CCMenuItemSpriteExtra::create(txtSetupGamemode, this, menu_selector(ChromaLayer::onShowPopup));
    m_btnSetupGamemode->setPosition(CCPoint(120.f - m_winSize.width / 2, m_winSize.height / 2 - 25.f));
    //m_btnSetupGamemode->setAnchorPoint(CCPoint(0.f, 0.5));
    hide(m_btnSetupGamemode, 0);
    m_btnSetupGamemode->setID("current-tab-label");
    m_btnSetupGamemode->setTag(3);
    menuSetup->addChild(m_btnSetupGamemode);

    auto txtSetupChannel = CCLabelBMFont::create("Channel", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    txtSetupChannel->setScale(0.36);
    txtSetupChannel->setID("label");
    this->m_btnSetupChannel = CCMenuItemSpriteExtra::create(txtSetupChannel, this, menu_selector(ChromaLayer::onShowPopup));
    m_btnSetupChannel->setPosition(CCPoint(60.f - m_winSize.width / 2, m_winSize.height / 2 - 45.f));
    //m_btnSetupChannel->setAnchorPoint(CCPoint(0.f, 0.5));
    hide(m_btnSetupChannel, 0);
    m_btnSetupChannel->setColor(ccc3(127, 127, 127));
    m_btnSetupChannel->setID("current-channel-label");
    m_btnSetupChannel->setTag(4);
    menuSetup->addChild(m_btnSetupChannel);

    this->m_scrollerSetupTabsEasy = ScrollLayerPlus::create(CCRect(0.f, 0.f, 110.f, 340.f));
    m_scrollerSetupTabsEasy->setAnchorPoint(CCPoint(0.5, 0.5));
	m_scrollerSetupTabsEasy->setPosition(CCPoint(-160.f, 0.f));
    m_scrollerSetupTabsEasy->ignoreAnchorPointForPosition(false);
	m_scrollerSetupTabsEasy->setContentSize(CCSize(120.f, 260.f));
	m_scrollerSetupTabsEasy->setID("easy-scroller");
    m_scrollerSetupTabsEasy->setTag(7);
    m_scrollerSetupTabsEasy->setVisible(false);
    m_scrollerSetupTabsEasy->setCeiling();
    menuSetup->addChild(m_scrollerSetupTabsEasy);

    this->m_scrollerSetupTabsAdv = ScrollLayerPlus::create(CCRect(0.f, 0.f, 110.f, 660.f));
    m_scrollerSetupTabsAdv->setAnchorPoint(CCPoint(0.5, 0.5));
	m_scrollerSetupTabsAdv->setPosition(CCPoint(-160.f, 0.f));
    m_scrollerSetupTabsAdv->ignoreAnchorPointForPosition(false);
	m_scrollerSetupTabsAdv->setContentSize(CCSize(120.f, 260.f));
	m_scrollerSetupTabsAdv->setID("advanced-scroller");
    m_scrollerSetupTabsAdv->setTag(15);
    m_scrollerSetupTabsAdv->setVisible(false);
    m_scrollerSetupTabsAdv->setCeiling();
    menuSetup->addChild(m_scrollerSetupTabsAdv);

    // add content
    float Y = 50.f;
    SetupItemCell* cell = nullptr;

    // effects
    for (int id = 15; id > 9; id--) {
        cell = SetupItemCell::create(id, Y, 16-id);
        static_cast<MyContentLayer*>(m_scrollerSetupTabsEasy->m_contentLayer)->addChild(cell);
        m_cells.push_back(cell);
        Y += 40.f;
    }
    // single icon
    cell = SetupItemCell::create(0, Y, 7);
    static_cast<MyContentLayer*>(m_scrollerSetupTabsEasy->m_contentLayer)->addChild(cell);
    m_cells.push_back(cell);
    Y = 50.f;

    // full icons
    for (int id = 15; id > 0; id--) {
        cell = SetupItemCell::create(id, Y, 16 - id);
        static_cast<MyContentLayer*>(m_scrollerSetupTabsAdv->m_contentLayer)->addChild(cell);
        m_cells.push_back(cell);
        Y += 40.f;
    }

    // workspace
    this->m_cellWorkspace = SetupOptionCell::create();
    menuSetup->addChild(m_cellWorkspace);
    m_cells.push_back(m_cellWorkspace);

    this->m_cellWaitspace = SetupOptionCell::create();
    menuSetup->addChild(m_cellWaitspace);
    m_cells.push_back(m_cellWaitspace);
    
    auto sprArwLeft = CCSprite::create("closeBtn.png"_spr);
    sprArwLeft->setScale(0.6);
    sprArwLeft->setID("channel-button-left");
    this->m_btnSetupArrowLeft = CCMenuItemSpriteExtra::create(sprArwLeft, this, menu_selector(ChromaLayer::onSwitchChannelPage));
    m_btnSetupArrowLeft->setPosition(CCPoint(-90.f, 0.f));
    m_btnSetupArrowLeft->setColor(ccc3(CELL_COLOR));
    m_btnSetupArrowLeft->setTag(1);
    hide(m_btnSetupArrowLeft, 0);
    m_btnSetupArrowLeft->setEnabled(false);
    menuSetup->addChild(m_btnSetupArrowLeft);

    auto sprArwRight = CCSprite::create("closeBtn.png"_spr);
    sprArwRight->setScale(0.6);
    sprArwRight->setID("channel-button-right");
    sprArwRight->setFlipX(true);
    this->m_btnSetupArrowRight = CCMenuItemSpriteExtra::create(sprArwRight, this, menu_selector(ChromaLayer::onSwitchChannelPage));
    m_btnSetupArrowRight->setPosition(CCPoint(190.f, 0.f));
    m_btnSetupArrowRight->setColor(ccc3(CELL_COLOR));    
    m_btnSetupArrowRight->setTag(2);
    hide(m_btnSetupArrowLeft, 0);
    m_btnSetupArrowRight->setEnabled(false);
    menuSetup->addChild(m_btnSetupArrowRight);

    // copy chroma setup
    auto sprCopy = CCSprite::create("copyBtn.png"_spr);
    sprCopy->setScale(0.8);
    sprCopy->setID("copy");
    this->m_btnSetupCopy = CCMenuItemSpriteExtra::create(sprCopy, this, menu_selector(ChromaLayer::onCopy));
    m_btnSetupCopy->setPosition(CCPoint(m_winSize.width / 2 - 30.f, m_winSize.height / 2 - 75.f));
    m_btnSetupCopy->setTag(5);
    m_btnSetupCopy->setColor(ccc3(CELL_COLOR));
    hide(m_btnSetupCopy, 0);
    menuSetup->addChild(m_btnSetupCopy);

    // paste chroma setup
    auto sprPaste = CCSprite::create("pasteBtn.png"_spr);
    sprPaste->setScale(0.8);
    sprPaste->setID("paste");
    this->m_btnSetupPaste = CCMenuItemSpriteExtra::create(sprPaste, this, menu_selector(ChromaLayer::onPaste));
    m_btnSetupPaste->setPosition(CCPoint(m_winSize.width / 2 - 30.f, m_winSize.height / 2 - 120.f));
    m_btnSetupPaste->setTag(6);
    m_btnSetupPaste->setColor(ccc3(CELL_COLOR));
    hide(m_btnSetupPaste, 0);
    menuSetup->addChild(m_btnSetupPaste);
    
    if (this->m_ptwo) {
        // set player
        for (auto cell : CCArrayExt<SetupItemCell*>(m_scrollerSetupTabsAdv->m_contentLayer->getChildren()))
            cell->switchPlayer();
        for (auto cell : CCArrayExt<SetupItemCell*>(m_scrollerSetupTabsEasy->m_contentLayer->getChildren()))
            cell->switchPlayer();        
    }

    // toggle on chroma
    for (auto cell : CCArrayExt<SetupItemCell*>(m_scrollerSetupTabsAdv->m_contentLayer->getChildren()))
        cell->toggleChroma(cell == m_currentTab);
    for (auto cell : CCArrayExt<SetupItemCell*>(m_scrollerSetupTabsEasy->m_contentLayer->getChildren()))
        cell->toggleChroma(cell == m_currentTab);

    this->m_hasSetupPage = true;
}

void ChromaLayer::makeColorPage() {
    // color menu
    auto menuColor = CCMenu::create();
    menuColor->setID("color-menu");
    this->addChild(menuColor);

    // title
    this->m_lbfColorTarget = CCLabelBMFont::create("Pick a Color", "ErasBold.fnt"_spr, 400.f, CCTextAlignment::kCCTextAlignmentCenter);
    m_lbfColorTarget->setPosition(CCPoint(0.f, 130.f));
    hide(m_lbfColorTarget, 0.35);
    m_lbfColorTarget->setTag(5);
    menuColor->addChild(m_lbfColorTarget);

    this->m_lbfColorItem = CCLabelBMFont::create("this color", "ErasBold.fnt"_spr, 300.f, CCTextAlignment::kCCTextAlignmentCenter);
    m_lbfColorItem->setPosition(CCPoint(0.f, 105.f));
    m_lbfColorItem->setColor(ccc3(192, 192, 192));
    hide(m_lbfColorItem, 0.2);
    m_lbfColorItem->setTag(6);
    menuColor->addChild(m_lbfColorItem);

    // pick color
    this->m_picker = CCControlColourPicker::colourPicker();
    m_picker->setPosition(CCPoint(-60.f, 0));
    m_picker->setContentSize(CCSize(0.f, 0.f));
    hide(m_picker, 0);
    m_picker->setID("color-picker");
    menuColor->addChild(m_picker);
    // red
    this->m_cellRed = ColorValueCell::create(0);
    menuColor->addChild(m_cellRed);
    m_cells.push_back(m_cellRed);
    // green
    this->m_cellGreen = ColorValueCell::create(1);
    menuColor->addChild(m_cellGreen);
    m_cells.push_back(m_cellGreen);
    // blue
    this->m_cellBlue = ColorValueCell::create(2);
    menuColor->addChild(m_cellBlue);
    m_cells.push_back(m_cellBlue);
    // hex
    this->m_cellHex = ColorHexCell::create();
    menuColor->addChild(m_cellHex);
    m_cells.push_back(m_cellHex);

    auto sprColorOri = ColorChannelSprite::create();
    this->m_btnColorDisplayOri = CCMenuItemSpriteExtra::create(sprColorOri, this, menu_selector(ChromaLayer::onColorDisplayBtn));
    m_btnColorDisplayOri->setPosition(CCPoint(-180.f, 30.f));
    hide(m_btnColorDisplayOri, 0);
    m_btnColorDisplayOri->setTag(1);
    menuColor->addChild(m_btnColorDisplayOri);

    auto sprColorCur = ColorChannelSprite::create();
    this->m_btnColorDisplayCur = CCMenuItemSpriteExtra::create(sprColorCur, this, menu_selector(ChromaLayer::onColorDisplayBtn));
    m_btnColorDisplayCur->setPosition(CCPoint(-180.f, -30.f));
    hide(m_btnColorDisplayCur, 0);
    m_btnColorDisplayCur->setTag(3);
    menuColor->addChild(m_btnColorDisplayCur);

    // arrow
    this->m_sprArrow = CCSprite::create("mysteriousArrow.png"_spr);
    m_sprArrow->setPosition(CCPoint(-180.f, 0.f));
    hide(m_sprArrow, 0.3);
    m_sprArrow->setColor(ccc3(CELL_COLOR));
    menuColor->addChild(m_sprArrow);

    // copy Ori
    auto sprCopyOri = CCSprite::create("copyBtn.png"_spr);
    sprCopyOri->setScale(0.8);
    sprCopyOri->setID("copy");
    this->m_btnColorCopyOri = CCMenuItemSpriteExtra::create(sprCopyOri, this, menu_selector(ChromaLayer::onCopy));
    m_btnColorCopyOri->setPosition(CCPoint(-180.f, 30.f));
    hide(m_btnColorCopyOri, 0);
    m_btnColorCopyOri->setTag(1);
    m_btnColorCopyOri->setColor(ccc3(CELL_COLOR));
    menuColor->addChild(m_btnColorCopyOri);

    // resc Ori
    auto sprRescOri = CCSprite::create("rescBtn.png"_spr);
    sprRescOri->setScale(0.75);
    sprRescOri->setID("resc");
    this->m_btnColorRescOri = CCMenuItemSpriteExtra::create(sprRescOri, this, menu_selector(ChromaLayer::onResc));
    m_btnColorRescOri->setPosition(CCPoint(-180.f, 30.f));
    m_btnColorRescOri->setTag(4);
    hide(m_btnColorRescOri, 0);
    m_btnColorRescOri->setColor(ccc3(CELL_COLOR));
    menuColor->addChild(m_btnColorRescOri);

    // copy Crt
    auto sprCopyCur = CCSprite::create("copyBtn.png"_spr);
    sprCopyCur->setScale(0.8);
    sprCopyCur->setID("copy");
    this->m_btnColorCopyCur = CCMenuItemSpriteExtra::create(sprCopyCur, this, menu_selector(ChromaLayer::onCopy));
    m_btnColorCopyCur->setPosition(CCPoint(-180.f, -30.f));
    m_btnColorCopyCur->setTag(3);
    hide(m_btnColorCopyCur, 0);
    m_btnColorCopyCur->setColor(ccc3(CELL_COLOR));
    menuColor->addChild(m_btnColorCopyCur);

    // paste Crt
    auto sprPasteCur = CCSprite::create("pasteBtn.png"_spr);
    sprPasteCur->setScale(0.8);
    sprPasteCur->setID("paste");
    this->m_btnColorPasteCur = CCMenuItemSpriteExtra::create(sprPasteCur, this, menu_selector(ChromaLayer::onPaste));
    m_btnColorPasteCur->setPosition(CCPoint(-180.f, -30.f));
    m_btnColorPasteCur->setTag(2);
    hide(m_btnColorPasteCur, 0);
    m_btnColorPasteCur->setColor(ccc3(CELL_COLOR));
    menuColor->addChild(m_btnColorPasteCur);

    this->m_hasColorPage = true;
}

void ChromaLayer::makeOptionsPage() {
    // options menu
    auto menuOptions = CCMenu::create();
    menuOptions->setID("options-menu");
    this->addChild(menuOptions);

    // optionsScroller
    // fullscreen scroll tolerance
    this->m_scrollerOptions = ScrollLayerPlus::create(CCRect(0.f, 0.f, 320.f, 400.f));
    m_scrollerOptions->setAnchorPoint(CCPoint(0.5, 0.5));
    m_scrollerOptions->ignoreAnchorPointForPosition(false);
	m_scrollerOptions->setContentSize(m_winSize);
    m_scrollerOptions->m_contentLayer->setPositionX(m_winSize.width / 2 - 160.f);
	m_scrollerOptions->setID("options-scroller");
    m_scrollerOptions->setVisible(false);
    menuOptions->addChild(m_scrollerOptions);

    float H = 50;
    int sTag = 0;

    // are you trying to find out what does this quesiton marks option mean here?
    sTag ++;
    auto questionMarkOpt = OptionTogglerCell::create("???", H, sTag, "???",
        "?????");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(questionMarkOpt);
    H += questionMarkOpt->getContentHeight() + 15.f;

    sTag ++;
    auto darkOpt = OptionTogglerCell::create("Dark Theme", H, sTag, "dark-theme",
        "Deep Dark Fantasy...");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(darkOpt);
    H += darkOpt->getContentHeight() + 15.f;

    sTag ++;
    auto blurOpt = OptionTogglerCell::create("Blur Background", H, sTag, "blur-bg",
        "Add a gaussian blur effect to the background. Give it up if you feel this effect can't worth your device lag.");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(blurOpt);
    H += blurOpt->getContentHeight() + 15.f;

    sTag ++;
    auto prevOpt = OptionTogglerCell::create("Preview Effects", H, sTag, "prev",
        "Preview Chroma Effects inside mod menu.");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(prevOpt);
    H += prevOpt->getContentHeight() + 15.f;

    sTag ++;
    auto uiTitle = OptionTitleCell::create("Interface Options", H, sTag, "interface-title");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(uiTitle);
    H += 40.f;

    sTag ++;
    auto sepglowOpt = OptionTogglerCell::create("Separate Glow Color Phase", H, sTag, "sep-glow",
        "Set phase (for Chromatic / Gradient mode) of Glow Color keeps 120 degrees delay from Main Color.\nOtherwise Glow Color aligns with Main Color");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(sepglowOpt);
    H += sepglowOpt->getContentHeight() + 15.f;

    sTag ++;
    auto sepsecondOpt = OptionTogglerCell::create("Separate Secondary Color Phase", H, sTag, "sep-second",
        "Set phase (for Chromatic / Gradient mode) of Secondary Color keeps 120 degrees lead from Main Color.\nOtherwise Secondary Color aligns with Main Color");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(sepsecondOpt);
    H += sepsecondOpt->getContentHeight() + 15.f;

    sTag ++;
    auto sepdualOpt = OptionTogglerCell::create("Separate Dual Mode Phase", H, sTag, "sep-dual",
        "Set color phase (for Chromatic / Gradient mode) of P2 keeps 180 degrees away from P1.\n"
        "Otherwise the two players just cycles the same phase.");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(sepdualOpt);
    H += sepdualOpt->getContentHeight() + 15.f;

    sTag ++;
    auto phaseTitle = OptionTitleCell::create("Phase Options", H, sTag, "phase-title");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(phaseTitle);
    H += 40.f;

    sTag ++;
    auto teleOpt = OptionTogglerCell::create("Align Spider TP Line", H, sTag, "tele-fix",
        "Not related to icons chroma though, this option will fix spider teleport jump (or just triggering a purple ring / pad) "
        "effect line strictly to the center point between the player positions before and after being teleported.");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(teleOpt);
    H += teleOpt->getContentHeight() + 15.f;

    sTag ++;
    auto ghostOpt = OptionTogglerCell::create("Original Ghost Trail", H, sTag, "dis-ghost",
        "Disable this mod's rewritten fixed Ghost Trail and apply RobTop's raw Ghost Trail Effect instead.\n"
        "But this will also result in ghost trail chroma not working.");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(ghostOpt);
    H += ghostOpt->getContentHeight() + 15.f;

    sTag ++;
    auto InitOpt = OptionTogglerCell::create("Chroma on Initial", H, sTag, "init",
        "Both PlayLayer and LevelEditorLayer may have a lag between the player happens and your player starts to move. "
        "If OFF, you can see your players of raw color when you pause quickly enough.");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(InitOpt);
    H += InitOpt->getContentHeight() + 15.f;

    sTag ++;
    auto fixTitle = OptionTitleCell::create("Fix Options", H, sTag, "fix-title");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(fixTitle);
    H += 40.f;

    sTag ++;
    auto editorOpt = OptionTogglerCell::create("Editor Test", H, sTag, "editor",
        "Apply to Editor Playtest. Will also add a button in your editor page if checked.\n"
        "But I do not promise your device will not lag there.");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(editorOpt);
    H += editorOpt->getContentHeight() + 15.f;

    sTag ++;
    auto riderOpt = OptionTogglerCell::create("Separate Riders", H, sTag, "rider",
        "If checked, the cube rider of ship / ufo / jetpack will follow Cube mode's color."
        "\nOtherwise she follow her vehicle's color.");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(riderOpt);
    H += riderOpt->getContentHeight() + 15.f;

    sTag ++;
    auto samedualOpt = OptionTogglerCell::create("Symmetric Chroma Pattern", H, sTag, "same-dual",
        "Let Player 2 follow P1's chroma pattern in playing.\n"
        "This only merges your chroma pattern, but their default colors may be different!\n"
        "NOT affecting menu preview.");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(samedualOpt);
    H += samedualOpt->getContentHeight() + 15.f;

    sTag ++;
    auto generalTitle = OptionTitleCell::create("General Options", H, sTag, "general-title");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(generalTitle);
    H += 40.f;

    sTag ++;
    auto switchOpt = OptionTogglerCell::create("Switch", H, sTag, "activate",
        "Master Switch of this mod.\n"
        "If disabled, all item icons (except current tab in setup page) in the mod menu will be set to grey-white and ignore Preview Effects option.");
    static_cast<MyContentLayer*>(m_scrollerOptions->m_contentLayer)->addChild(switchOpt);
    H += switchOpt->getContentHeight() + 15.f;

    m_scrollerOptions->m_contentLayer->setContentHeight(H - 10.f);
    m_scrollerOptions->setTag(sTag);
    m_scrollerOptions->setCeiling();

    this->m_hasOptionsPage = true;
}

void ChromaLayer::makeInfoPage() {
    // info menu
    auto menuInfo = CCMenu::create();
    menuInfo->setID("info-menu");
    this->addChild(menuInfo);

    auto lbfName = CCLabelBMFont::create("Chroma Icons", "ErasBold.fnt"_spr, 200, CCTextAlignment::kCCTextAlignmentCenter);
    lbfName->setPosition(CCPoint(0.f, 130.f));
    hide(lbfName, 0.32);
    lbfName->setID("about");
    menuInfo->addChild(lbfName);

    auto lbfVersion = CCLabelBMFont::create(Mod::get()->getVersion().toVString().c_str(), "ErasBold.fnt"_spr, 150, CCTextAlignment::kCCTextAlignmentCenter);
    lbfVersion->setPosition(CCPoint(0.f, 110.f));
    hide(lbfVersion, 0.15);
    lbfVersion->setColor(ccc3(127, 127, 127));
    lbfVersion->setID("version");
    menuInfo->addChild(lbfVersion);

    auto lbfManual = CCLabelBMFont::create("About This Mod", "ErasBold.fnt"_spr, 300, CCTextAlignment::kCCTextAlignmentCenter);
    lbfManual->setPosition(CCPoint(0.f, 80.f));
    hide(lbfManual, 0.25);
    lbfManual->setID("manual");
    menuInfo->addChild(lbfManual);

    auto lbfAuthor = CCLabelBMFont::create("Website / Contact", "ErasBold.fnt"_spr, 300, CCTextAlignment::kCCTextAlignmentCenter);
    lbfAuthor->setPosition(CCPoint(0.f, 10.f));
    hide(lbfAuthor, 0.25);
    lbfAuthor->setID("author");
    menuInfo->addChild(lbfAuthor);

    auto menuManual = CCMenu::create();
    menuManual->setID("manual-menu");
    menuManual->setPosition(CCPoint(0.f, 50.f));
    menuManual->setAnchorPoint(CCPoint(0.5, 0.5));
    menuManual->ignoreAnchorPointForPosition(false);
    menuManual->setContentSize(CCSize(300.f, 20.f));
    menuInfo->addChild(menuManual);

    int index = 0;
    std::vector<const char*> manuals = {"geodeBtn.png"_spr, "githubBtn.png"_spr};
    for (auto manual : manuals) {
        auto spr = CCSprite::create(manual);
        spr->setScale(0.6);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(ChromaLayer::onInfoButtons));
        btn->setColor(ccc3(CELL_COLOR));
        btn->setTag(index);
        hide(btn, 0);
        menuManual->addChild(btn);
        index ++;
    }
    menuManual->setLayout(AxisLayout::create()
        ->setGap(10.f)
        ->setAxisAlignment(AxisAlignment::Center)
    );
    menuManual->updateLayout();

    auto menuAuthor = CCMenu::create();
    menuAuthor->setID("author-menu");
    menuAuthor->setPosition(CCPoint(0.f, -20.f));
    menuAuthor->setAnchorPoint(CCPoint(0.5, 0.5));
    menuAuthor->ignoreAnchorPointForPosition(false);
    menuAuthor->setContentSize(CCSize(300.f, 20.f));
    menuInfo->addChild(menuAuthor);

    index = 0;
    std::vector<const char*> authors = {"youtubeBtn.png"_spr, "twitterBtn.png"_spr, "discordBtn.png"_spr};
    // what's this?
    if (Loader::get()->isModLoaded("endless-spike-studio.endless-services-connector"))
        authors.push_back("bilibiliBtn.png"_spr);
    for (auto author : authors) {
        auto spr = CCSprite::create(author);
        spr->setScale(0.6);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(ChromaLayer::onInfoButtons));
        btn->setColor(ccc3(CELL_COLOR));
        btn->setTag(10 + index);
        hide(btn, 0);
        menuAuthor->addChild(btn);
        index ++;
    }
    menuAuthor->setLayout(AxisLayout::create()
        ->setGap(10.f)
        ->setAxisAlignment(AxisAlignment::Center)
    );
    menuAuthor->updateLayout();

    auto lbfCreditTitle = CCLabelBMFont::create("Special Thanks", "ErasBold.fnt"_spr, 200.f, CCTextAlignment::kCCTextAlignmentCenter);
    lbfCreditTitle->setPosition(CCPoint(0.f, -60.f));
    hide(lbfCreditTitle, 0.25);
    lbfCreditTitle->setID("thanks-title");
    menuInfo->addChild(lbfCreditTitle);

    auto lbfCreditContent = CCLabelBMFont::create(
        "@hiimjustin000 for More Icons mod support & mac Reverse Engineering\n"
        "@Mat for index login, CCSequence mac build\n"
        "@TheSillyDoggo for Blur Background\n"
        "@irryan for internal mod test debug\n"
        "@clunos for mac mod test",
        "ErasBold.fnt"_spr, 400.f, CCTextAlignment::kCCTextAlignmentCenter);
    lbfCreditContent->setPosition(CCPoint(0.f, -110.f));
    lbfCreditContent->setColor(ccc3(127, 127, 127));
    hide(lbfCreditContent, 0.16);
    lbfCreditContent->setID("thanks-content");
    menuInfo->addChild(lbfCreditContent);

    this->m_hasInfoPage = true;
}