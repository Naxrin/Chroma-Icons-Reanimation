#include "Layer.hpp"
#include <random>

// menu type identifier
enum class LayerType {
    PlayLayer,
    LevelEditorLayer,
    MenuLayer
};

// allow chroma
static LayerType layerType = LayerType::MenuLayer;
// level
static GJGameLevel* Level;
// get level progress
static float progress;
// get current percantage
static float percentage;
// phase for game layer update
// mod menu has its own phase standalone
static float lvlphase;
// reset
//extern std::vector<int> reset2;
// reset
extern std::map<PlayerObject*, int> reset;
// bools
extern std::map<std::string, bool> opts;
// speed option
extern float speed;
// setup center
extern std::map<short, ChromaSetup> setups;

// garage button
#include <Geode/modify/GJGarageLayer.hpp>
class $modify(IconLayer, GJGarageLayer) {
	bool init(){
		if (!GJGarageLayer::init())
            return false;

        if (Mod::get()->getSavedValue<bool>("hide-garage"))
            return true;
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto menu = this->getChildByID("shards-menu");
        menu->setContentSize(CCSize(40.f, 260.f));
        menu->setPositionY(winSize.height/2-25.f);

        if (menu){
            auto img = CCSprite::create("rgbicon.png"_spr);
            img->setScale(0.6);
            img->setRotation(10.f);

            auto btn = CircleButtonSprite::create(img, CircleBaseColor::Gray, CircleBaseSize::Small);
            auto button = CCMenuItemSpriteExtra::create(btn, this, menu_selector(IconLayer::onChromaMenu));
            button->setID("chroma_icons_entry"_spr);
            
            menu->addChild(button);
            menu->updateLayout();
        }
        else
            log::warn("The button wants to lay in Garage Menu but he failed to find the shards menu");
		return true;
	}

	void onChromaMenu(CCObject *sender) {
		ChromaLayer::create()->show();
	}
};

// pause menu button
#include <Geode/modify/PauseLayer.hpp>
class $modify(PoseLayer, PauseLayer) {
	void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        // button
        auto img = CCSprite::create("rgbicon.png"_spr);
        img->setScale(0.6);
        auto button = CCMenuItemSpriteExtra::create(img, this, menu_selector(PoseLayer::onChromaMenu));//menu_selector(IconLayer::onColorButton)
        button->setID("chroma_icons_central"_spr);
        
        if (auto target = this->getChildByID("right-button-menu")) {
            target->addChild(button);
            target->updateLayout();
        }
	}

    // call player objects to refresh
	void onChromaMenu(CCObject *) {
        ChromaLayer::create()->show();
	}
};

// editor ui button
// guess why not directly modify LevelEditorLayer?
#include <Geode/modify/EditorUI.hpp>
class $modify(EditorMenu, EditorUI) {
    struct Fields {
        CCMenuItemSpriteExtra* m_chromaBtn = nullptr;
    };
    bool init(LevelEditorLayer* parent) {
        if (!EditorUI::init(parent))
            return false;
        // find the menu
        auto menu = this->getChildByID("undo-menu");
        if (!(menu && opts["editor"]))
            return true;

        // add button
        auto img = CCSprite::create("rgbicon.png"_spr);
        img->setScale(0.6);

        m_fields->m_chromaBtn = CCMenuItemSpriteExtra::create(img, this, menu_selector(EditorMenu::onChromaMenu));
        m_fields->m_chromaBtn->setID("chroma_icons_entry"_spr);
        m_fields->m_chromaBtn->setPosition(CCPoint(163.f, 19.75));

        menu->addChild(m_fields->m_chromaBtn);
        menu->updateLayout();
		return true;        
    }

    // why doesn't robtop directly hide his undo-menu?
    // i have no words
    void showUI(bool show) {
        EditorUI::showUI(show);
        if (m_fields->m_chromaBtn)
            m_fields->m_chromaBtn->setVisible(show);
    }

	void onChromaMenu(CCObject *sender) {
		ChromaLayer::create()->show();
	}
};

// mainly global phase operation
#include <Geode/modify/PlayLayer.hpp>
class $modify(GameLayer, PlayLayer) {
    // reset phase and tell mod central the level type in init
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        // reset phase
        lvlphase = 0;
        // percentage = 0
        percentage = 0;
        // register level
        Level = level;
        // allow chroma
        layerType = LayerType::PlayLayer;
        reset.clear();
        // log
        progress = level->m_normalPercent.value();
        return PlayLayer::init(level, useReplay, dontCreateObjects);
    }
    // edit phase
    void postUpdate(float d) override {
        // iterate phase and update progress
        lvlphase = fmod(lvlphase + 360 * d * speed, 360.f);
        percentage = this->getCurrentPercent();
        PlayLayer::postUpdate(d);
    }

    void resetPlayer() {
        PlayLayer::resetPlayer();
        progress = m_level->m_normalPercent.value();
    }
};

// I'm afraid of typeinfo cast so I decide not to directly modify GJBaseGameLayer
// also menu layer has a GJBaseGameLayer, so it's even more horrible
#include <Geode/modify/LevelEditorLayer.hpp>
class $modify(NivelEditorLayer, LevelEditorLayer) {
    // reset phase and tell mod central the level type in init
	bool init(GJGameLevel *level, bool p) {
        // reset phase
        lvlphase = 0;
        // percentage = 0
        percentage = 0;
        // register level
        Level = level;
        // allow chroma
        layerType = LayerType::LevelEditorLayer;
        reset.clear();
        // log
        progress = level->m_normalPercent.value();
		return LevelEditorLayer::init(level, p);
    }

    // edit phase
    void postUpdate(float d) override {
        if (opts["editor"])
            lvlphase = fmod(lvlphase + 360 * d * speed, 360.f);
        LevelEditorLayer::postUpdate(d);
    }

    void resetPlayer() {
        LevelEditorLayer::resetPlayer();
        progress = m_level->m_normalPercent.value();
    }
};

// I'm afraid of typeinfo cast so I decide not to directly modify GJBaseGameLayer
// also menu layer has a GJBaseGameLayer, so it's even more horrible
#include <Geode/modify/MenuGameLayer.hpp>
class $modify(MainGameLayer, MenuGameLayer) {
    // reset phase and tell mod central the level type in init
	bool init() override {
        // reset phase
        lvlphase = 0;
        // allow chroma
        layerType = LayerType::MenuLayer;
        reset.clear();
		return MenuGameLayer::init();
    }

    // edit phase
    void update(float d) override {
        if (opts["???"])
            lvlphase = fmod(lvlphase + 360 * d * speed, 360.f);

        MenuGameLayer::update(d);
    }

    void resetPlayer() {
        if (opts["???"])
            progress = std::rand() / 100;
        MenuGameLayer::resetPlayer();
    }
};

// My god, this mod finally starts to chroma your icons as definitely expected from here on
#include <Geode/modify/PlayerObject.hpp>
class $modify(ChromaPlayer, PlayerObject) {
    // record this player's pointer
    bool init(int p0, int p1, GJBaseGameLayer *p2, cocos2d::CCLayer *p3, bool p4) {
        reset[this] = false;
        return PlayerObject::init(p0, p1, p2, p3, p4);
    }

    // update override, chroma icons regarding current setup
    void update(float d) override {
        // base
        this->PlayerObject::update(d);

        // color test
        //m_colorRelated = ccc3(100, 100, 100);
        //m_colorRelated2 = ccc3(200, 200, 200);
        //m_secondColorRelated = ccc3(50, 50, 50);
        //m_flashRelated3 = ccc3(150, 150, 150);
        //return;
        // only chroma the visible two
        if ((!opts["activate"] && !reset[this]) || !this->isVisible() || this->m_isDead)
            return;
        if (layerType == LayerType::LevelEditorLayer && !opts["editor"])
            return;
        if (layerType == LayerType::MenuLayer) {
            if (opts["???"])
                percentage = 100 * this->getPositionX() / CCDirector::sharedDirector()->getWinSize().width;
            else
                return;
        }
        // get some phase offsets
        // as sending negative phase value to getChroma(...) is not allowed, all of them should really be positive
        bool p = this->m_isSecondPlayer && !opts["same-dual"];
        short od = this->m_isSecondPlayer && (opts["sep-dual"]) ? 180 : 0;
        short o2 = opts["sep-second"] ? 120 : 0;
        short o3 = opts["sep-glow"] ? 240 : 0;
        // easy-mode
        short id = this->getStatusID();

        // get the chroma pattern result firstly
        ccColor3B main = getChroma(setups[getIndex(p, id, 0)],
            m_playerColor1, lvlphase + od, percentage, progress, reset[this]);
        ccColor3B secondary = getChroma(setups[getIndex(p, id, 1)],
            m_playerColor2, lvlphase + od + o2, percentage, progress, reset[this]);
        ccColor3B glow = getChroma(setups[getIndex(p, id, 2)],
            m_glowColor, lvlphase + od + o3, percentage, progress, reset[this]);
        ccColor3B white = getChroma(setups[getIndex(p, id, 3)],
            ccc3(255, 255, 255), lvlphase + od, percentage, progress, reset[this]);

        // icons
        // for compatibility with Seperate dual icons or other mods
        // here I should always avoid calling their member functions
        bool isRider = false;

        // icons with vehicles
        if (m_isShip || m_isBird) {
            isRider = opts["rider"] && id;
            // icon
            this->m_iconSprite->setColor(isRider ? getChroma(setups[getIndex(p,  1, 0)],
                m_playerColor1, lvlphase + od, percentage, reset[this]) : main);
            this->m_iconSpriteSecondary->setColor(isRider ? getChroma(setups[getIndex(p, 1, 1)],
                m_playerColor2, lvlphase + od + o2, percentage, reset[this]) : secondary);
            if (m_hasGlow)
                this->m_iconGlow->setColor(isRider ? getChroma(setups[getIndex(p, 1, 2)],
                    m_glowColor, lvlphase + od + o3, percentage, reset[this]) : glow);
            this->m_iconSpriteWhitener->setColor(isRider ? getChroma(setups[getIndex(p, 1, 3)],
                ccc3(255, 255, 255), lvlphase + od, percentage, reset[this]) : white);
            // vehicle
            this->m_vehicleSprite->setColor(main);
            this->m_vehicleSpriteSecondary->setColor(secondary);
            if (m_hasGlow)
                this->m_vehicleGlow->setColor(glow);
            this->m_vehicleSpriteWhitener->setColor(white);
        }
        // robot
        else if (m_isRobot) {
            auto arr = this->m_robotSprite->getChildByType<CCPartAnimSprite>(0)->m_spriteParts;
            for (auto part : CCArrayExt<CCSpritePart*>(arr)) {
                part->setColor(main);
                part->getChildByType<CCSprite>(0)->setColor(secondary);
                if (part->getTag() == 1)
                    part->getChildByType<CCSprite>(1)->setColor(white);
            }
            if (m_hasGlow)
                for (auto spr: CCArrayExt<CCSprite*>(this->m_robotSprite->getChildByType<CCPartAnimSprite>(0)->getChildByType<CCSprite>(0)->getChildren()))
                    spr->setColor(glow);
        }
        // spider
        else if (m_isSpider) {
            auto arr = this->m_spiderSprite->getChildByType<CCPartAnimSprite>(0)->m_spriteParts;
            for (auto part : CCArrayExt<CCSpritePart*>(arr)) {
                part->setColor(main);
                part->getChildByType<CCSprite>(0)->setColor(secondary);
                if (part->getTag() == 1)
                    part->getChildByType<CCSprite>(1)->setColor(white);
            }
            if (m_hasGlow)
                for (auto spr: CCArrayExt<CCSprite*>(this->m_spiderSprite->getChildByType<CCPartAnimSprite>(0)->getChildByType<CCSprite>(0)->getChildren()))
                    spr->setColor(glow);
        }
        // regular modes
        else {
            this->m_iconSprite->setColor(main);
            this->m_iconSpriteSecondary->setColor(secondary);
            if (m_hasGlow)
                this->m_iconGlow->setColor(glow);
            this->m_iconSpriteWhitener->setColor(white);
        }

        // trail
        if (m_playerStreak != 2 && m_playerStreak != 7)
            this->m_regularTrail->setColor(getChroma(setups[getIndex(p, 11, id + 4)],
                m_playerColor2, lvlphase + od, percentage, reset[this]));
        // wave trail
        if (this->m_isDart)
            this->m_waveTrail->setColor(getChroma(setups[getIndex(p, 12)],
                gm->getGameVariable("0096") ? m_playerColor2 : m_playerColor1, lvlphase + od, percentage, reset[this]));
        // dash fire
        if (this->m_isDashing)
            this->m_dashFireSprite->setColor(getChroma(setups[getIndex(p, 13, id + 4)],
                gm->getGameVariable("0062") ? m_playerColor1 : m_playerColor2, lvlphase + od, percentage, reset[this]));
        // ufo shell
        if (this->m_isBird)
            this->m_birdVehicle->setColor(getChroma(setups[getIndex(p, 15)],
                ccc3(255, 255, 255), lvlphase + od, percentage, reset[this]));

        // confirm it's really reset
        if (reset[this])
            reset[this] --;
    }

    // spider teleport line i guess
    // this is not for mac now
    #ifndef GEODE_IS_MACOS
    void spiderTestJumpInternal(bool unk) {

        if (!opts["activate"]) {
            PlayerObject::spiderTestJump(unk);
            return;
        }

        auto pori = this->getPosition();
        PlayerObject::spiderTestJumpInternal(unk);
        auto pcur = this->getPosition();
        
        for (auto node : CCArrayExt<CCNode*>(this->getParent()->getChildren())) {
            // stupid judge
            if (auto tele = typeinfo_cast<CCSprite*>(node)) {
                // confirm class type
                if (typeinfo_cast<PlayerObject*>(node) || !(tele->getContentSize() == CCSize(221.f, 21.f) )) // && tele->getRotation() == (this->m_isUpsideDown ? 90.f : -90.f)
                    continue;
                // confirm position
                if (detectTPline(tele, pori, pcur))
                    tele->setColor(getChroma(setups[getIndex(this->m_isSecondPlayer && !opts["same-dual"], 14, this->getStatusID() + 4)],
                        m_playerColor1, lvlphase + ((this->m_isSecondPlayer && opts["sep-dual"]) ? 180.f : 0), percentage, progress, reset[this]));
            }
        }
    }

    bool detectTPline(CCSprite *tele, CCPoint &pori, CCPoint &pcur) {
        //log::error("isSideWay = {}", m_isSideways);
        CCPoint pos = tele->getPosition();
        CCPoint posfix;
        // horizental tp
        if (this->m_isSideways) {
            posfix = CCPoint((pori.x + pcur.x) / 2, pori.y);
            //log::error("delta x = {} y = {}", pos.x - posfix.x + (this->m_isGoingLeft ? 4.5 : -10.5), pos.y - posfix.y);
            if (std::abs(pos.y - posfix.y) > 0.001 || std::abs(pos.x - posfix.x + (this->m_isGoingLeft ? 4.5 : -10.5)) > 0.001
                || tele->getRotation() != this->m_isUpsideDown * 180.f)
                return false;
        }
        // vertical tp
        else {
            posfix = CCPoint(pori.x, (pori.y + pcur.y) / 2);
            //log::error("delta x = {} y = {}", pos.x - posfix.x + (this->m_isGoingLeft ? 7.5 : -7.5), pos.y - posfix.y);
            if (std::abs(pos.x - posfix.x + (this->m_isGoingLeft ? 7.5 : -7.5)) > 0.001 || std::abs(pos.y - posfix.y) > 0.001
                || tele->getRotation() != (this->m_isUpsideDown ? 90.f : -90.f))
                return false;
        }
        // confirmed
        if (opts["tele-fix"])
            tele->setPosition(posfix);
        return true;
    }
    #endif

    short getStatusID() {
        if (opts["easy"]) return 0;
        if (this->m_isShip) return Level && Level->isPlatformer() ? 9 : 2;
        else if (this->m_isBall) return 3;
        else if (this->m_isBird) return 4;
        else if (this->m_isDart) return 5;
        else if (this->m_isRobot) return 6;
        else if (this->m_isSpider) return 7;
        else if (this->m_isSwing) return 8;
        return 1;
    }
};

// control default values
$on_mod(Loaded) {
    // load setups
    for (short p = 0; p < 2; p++) {
        // icons
        for (short id = 0; id < 10; id++)
            for (short chnl = 0; chnl < 4; chnl++)
                setups[getIndex(p, id, chnl)] = Mod::get()->getSavedValue<ChromaSetup>(getConfigKey(p, id, chnl), DEFAULT_SETUP);
        // effects
        for (short id = 11; id < 16; id++) {
            if (id == 12 || id == 15)
                setups[getIndex(p, id, 4)] = Mod::get()->getSavedValue<ChromaSetup>(getConfigKey(p, id, 4), DEFAULT_SETUP);
            else
                for (short chnl = 4; chnl < 14; chnl++)
                    setups[getIndex(p, id, chnl)] = Mod::get()->getSavedValue<ChromaSetup>(getConfigKey(p, id, chnl), DEFAULT_SETUP);
        }
    }
    // speed
    speed = Mod::get()->getSavedValue<float>("speed", 1);
    // load options
    std::map<std::string, bool> defaultOpts = {
        {"easy", true},
        {"activate", true},
        {"same-dual", false},
        {"rider", false},
        {"editor", false},
        {"tele-fix", false},
        {"sep-dual", false},
        {"sep-second", false},
        {"sep-glow", false},
        {"prev", true},
        {"blur-bg", false},
        {"dark-theme", true},
        {"???", false}
    };
    for (auto [key, val] : defaultOpts)
        opts[key] = Mod::get()->getSavedValue<bool>(key, val);
}