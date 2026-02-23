#pragma once

#include "include/cells.hpp"

#include <Geode/ui/Popup.hpp>
#include "BlurAPI.hpp"

// interface code in menu
enum class Page {
    Terminal, Init, Item, Setup, Color, Options, Info, Popup, Warn
};

// hide the node in init and show them up in menu launch
template<class T>
inline void hide(T node, float scale) {
    node->setScale(scale);
    node->setOpacity(0);
    node->setVisible(false);
}

// hide the ndoe in init, but scaleX != scaleY
template<class T>
inline void hide(T node, float scaleX, float scaleY) {
    node->setScaleX(scaleX);
    node->setScaleY(scaleY);
    node->setOpacity(0);
    node->setVisible(false);
}

// fubao is like a clever neko witch
// miss her super much
class ChromaLayer : public Popup, public ColorPickerDelegate {
protected:
    /********** VARIENT CENTRAL ***********/

    // winSize
    const CCSize m_winSize = CCDirector::sharedDirector()->getWinSize();

    // interface status chain
    // @note for initialize the first value is Init, when you're gonna quit the menu we temply add a Terminal page
    // @note but we should know both Init and Terminal are virtual fake
    // @note if warn page is triggered, warn page will be added firstly
    std::vector<Page> m_pages;

    // already has Item Page
    bool m_hasItemPage;
    // already has Setup Page
    bool m_hasSetupPage;
    // already has Color Page
    bool m_hasColorPage;
    // already has Options Page
    bool m_hasOptionsPage;
    // already has Hint Page
    bool m_hasHintPage;
    // already has Info Page
    bool m_hasInfoPage;

    // dual player 1/2
    bool m_ptwo;

    // the identifier of the current modifying icon/effect for setup menu
    // @note easy mode : common = 0
    // @note adv mode : icons = 1-9
    // @note Effects (11~15) : trail, wave trail, dash fire, teleport line, ufo shell...
    int m_tab;

    // current gamemode in setup page
    // @note icon-jetpack = 0-9
    Gamemode m_gamemode;

    // record adv mode gamemode when switching to ez mode
    Gamemode m_gamemodeAdv;

    // current item channel in setup page
    // @note main/second/glow/white
    Channel m_channel = Channel::Main;

    // current editing color tag in case color page dosen't know what he's working on
    int m_colorTag = 0;

    // phase for schedule update
    float m_phase = 0;
    // percentage simulator
    float m_percentage = 0;

    // current config data
    ChromaSetup m_currentSetup = DEFAULT_SETUP;

    // current color
    // @note varient inside color page
    ccColor3B m_oriColor;
    // original color
    // @note fixed inside color page
    ccColor3B m_crtColor;

    // setup scroller
    SetupItemCell* m_currentTab = nullptr;

    std::pair<bool, ccColor3B> m_clipColor;
    std::pair<bool, ChromaSetup> m_clipSetup;

    // mute onClose if called
    bool m_onSlider = false;

    /********** UI THINGS ***********/

    // background
    CCLayerColor* m_bg = nullptr;

    // all cell nodes to switch theme (except option cells)
    CCArrayExt<BaseCell*> m_cells;

    // exit button
    CCMenuItemSpriteExtra* m_btnExit;
    // info button
    CCMenuItemSpriteExtra* m_btnInfo;
    // apply button
    CCMenuItemSpriteExtra* m_btnApply;
    // options button
    CCMenuItemSpriteExtra* m_btnOptions;
    // mode button
    CCMenuItemToggler* m_btnMode;

    // warning
    WarnCell* m_cellWarning;
    // title
    TitleCell* m_cellTitle;
    // player switch
    CCMenuItemToggler* m_btnItemPlayer;
    // full icons
    ItemCell* m_cellItemAdv;
    // single icon
    ItemCell* m_cellItemEasy;
    // effect
    ItemCell* m_cellItemEffect;

    // setup player switch
    CCMenuItemToggler* m_btnSetupPlayer;
    // setup item label
    CCMenuItemSpriteExtra* m_btnSetupGamemode;
    // setup channel label
    CCMenuItemSpriteExtra* m_btnSetupChannel;
    // setup adv items scroller
    ScrollLayerPlus* m_scrollerSetupTabsAdv;
    // setup easy items scroller
    ScrollLayerPlus* m_scrollerSetupTabsEasy;
    // main setup workspace
    SetupOptionCell* m_cellWorkspace;
    // temp setup space
    SetupOptionCell* m_cellWaitspace;
    // setup channel switch left
    CCMenuItemSpriteExtra* m_btnSetupArrowLeft;
    // setup channel switch right
    CCMenuItemSpriteExtra* m_btnSetupArrowRight;
    // copy button
    CCMenuItemSpriteExtra* m_btnSetupCopy;
    // paste button
    CCMenuItemSpriteExtra* m_btnSetupPaste;

    // display what id and channel he is working on
    CCLabelBMFont* m_lbfColorTarget;
    // display what color he is now configuring
    CCLabelBMFont* m_lbfColorItem;
    // original color displayer
    CCMenuItemSpriteExtra* m_btnColorDisplayOri;
    // current color displayer
    CCMenuItemSpriteExtra* m_btnColorDisplayCur;
    // arrow
    CCSprite* m_sprArrow;
    // picker
    CCControlColourPicker* m_picker;
    // R val
    ColorValueCell* m_cellRed;
    // G val
    ColorValueCell* m_cellGreen;
    // B val
    ColorValueCell* m_cellBlue;
    // HEX val
    ColorHexCell* m_cellHex;

    // copy ori
    CCMenuItemSpriteExtra* m_btnColorCopyOri;
    // resc ori
    CCMenuItemSpriteExtra* m_btnColorRescOri;
    // copy crt
    CCMenuItemSpriteExtra* m_btnColorCopyCur;
    // resc paste
    CCMenuItemSpriteExtra* m_btnColorPasteCur;

    // option scroller
    ScrollLayerPlus* m_scrollerOptions;

    // hint content
    CCLabelBMFont* m_lbfHintContent;

    // radios
    std::vector<ListenerHandle> m_radios;

    /*********** INITIAL SETUP ***********/

    // makup initial UI
    bool init() override;

    // initial radios
    void installRadios();

    // make item page
    void makeItemPage();

    // make setup page
    void makeSetupPage();

    // make color page
    void makeColorPage();

    // make options page
    void makeOptionsPage();

    // make info page
    void makeInfoPage();

    /*********** UTILITY ***********/

    // override update
    void update(float xdt) override;

    // get color target
    // @return point to the color button who calls color page
    CCMenuItemSpriteExtra* getColorTarget();

    //refresh some chroma preview status when setup changed
    void dumpConfig();

    // save config -> switch current ID and refresh setup page
    // @param tab new target id he will switch to.
    // @return true if the value is really changed
    bool switchTab(int tab);

    // make a hint popup
    void makeHintPopup(std::string title, std::string content);


    // ColorPickerDelegate function override
    // @param color new color from the picker
    void colorValueChanged(ccColor3B color) override;

    /*********** TRANSITION ***********/

    // switch theme color between dark and white
    void switchTheme();

    // fade in or out main menu buttons in the four corners
    //void fadeMainMenuButtons();

    // refresh color page status
    // @param type 0->init 1->rgb 2->hex 3->circle 4->restore
    void refreshColorPage(int type);

    // show & hide color sub options
    // @param isCrt whether the pressed sprite is current color (true) or original color (false)
    // @param display to show or to hide after clicked
    void transistColorBtn(bool isCrt, bool display);

    // run main ui animation
    void fadeMainMenu();

    // run warning page animation
    void fadeWarnPage();

    // run item select page
    void fadeItemPage();

    // run setup ui animation
    void fadeSetupPage();

    // run pickcolor ui animation
    void fadeColorPage();

    // run options ui animation
    void fadeOptionsPage();

    // run popup ui animation
    void fadePopupPage();

    // run info ui animation
    void fadeInfoPage();

    /*************** CALLBACK ***************/

    // switch player
    // @param sender m_playerBtn1->tag = 4 m_playerBtn2->tag = 2
    void onSwitchPlayer(CCObject* sender);

    // on switch easy/advanced mode
    // @param sender the easy adv button
    void onSwitchEasyAdv(CCObject* sender);

    // show popup regarding sender tag and current page
    void onShowPopup(CCObject* sender);

    // flip channel page
    // @param sender m_leftArrowSetupBtn->tag = 4 m_rightArrowSetupBtn->tag = 2
    void onSwitchChannelPage(CCObject* sender);

    // click the options button
    // @param sender the options button
    void onOptionsPage(CCObject* sender);

    // launch info page
    // @param sender the options button
    void onInfoPage(CCObject* sender);

    // clicked an info button
    // @param sender the options button
    void onInfoButtons(CCObject* sender);

    // expand or collapse sub buttons
    void onColorDisplayBtn(CCObject* sender);

    // copy setup / color
    // @param sender copy button's tags are important signal
    // @note copy button 1->ori 3->crt 5->setup
    void onCopy(CCObject* sender);

    // paste setup / color
    // @param sender paste button's tags are important signal
    // @note paste button 4->crt 6->setup
    void onPaste(CCObject* sender);

    // resc color
    // @param sender resc button
    // @note resc button 2->ori
    void onResc(CCObject* sender);

    // apply setup / color
    // @param sender apply button
    void onApply(CCObject* sender);

    // override onClose feedback
    // @param sender actually not important
    void onClose(CCObject*) override;
public:
    // hey anyone will use it?
    const std::string CLASS_NAME = "ChromaLayer";

    // process some init animation
    void show() override;

    // not knowing why android directly leave mod menu
    void keyBackClicked() override {
        this->onClose(nullptr);
    }
    // constructor
    static ChromaLayer* create() {
        auto layer = new ChromaLayer();

        if (layer && layer->init()) {
            layer->autorelease();
            return layer;
        };
        CC_SAFE_DELETE(layer);
        return nullptr;
    }
};