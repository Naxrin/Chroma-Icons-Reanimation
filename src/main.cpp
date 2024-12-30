#include "Layer.hpp"

// allow chroma
static bool allow_chroma = false;
// level
static GJGameLevel* Level;
// get level progress
static float percentage;
// phase for game layer update
// mod menu has its own phase standalone
static float lvlphase;
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
        allow_chroma = true;
        return PlayLayer::init(level, useReplay, dontCreateObjects);
    }
    // edit phase
    void postUpdate(float d) override {
        // iterate phase and update progress
        lvlphase = fmod(lvlphase + 360 * d * speed, 360.f);
        percentage = this->getCurrentPercent();
        PlayLayer::postUpdate(d);
    }

    // quit
    ~GameLayer() {
        allow_chroma = false;
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
        allow_chroma = true;
		return LevelEditorLayer::init(level, p);
    }

    // edit phase
    void postUpdate(float d) override {
        if (opts["editor"])
            lvlphase = fmod(lvlphase + 360 * d * speed, 360.f);
        LevelEditorLayer::postUpdate(d);
    }

    // quit
    ~NivelEditorLayer() {
        allow_chroma = false;
    }
};

// My god, this mod finally starts to chroma your icons as definitely expected from here on
#include <Geode/modify/PlayerObject.hpp>
class $modify(ChromaPlayer, PlayerObject) {
    // record their default color
    struct Fields {
        ccColor3B main;
        ccColor3B secondary;
        ccColor3B glow;
    };

    bool init(int p0, int p1, GJBaseGameLayer *p2, cocos2d::CCLayer *p3, bool p4) {
        reset[this] = false;
        return PlayerObject::init(p0, p1, p2, p3, p4);
    }

    // update override, chroma icons regarding current setup
    void update(float d) override {
        // base
        this->PlayerObject::update(d);
        // only chroma the visible two
        if ((!opts["activate"] && !reset[this]) || !allow_chroma || !this->isVisible() || this->m_isDead)
            return;
        // get some phase offsets
        // as sending negative phase value to getChroma(...) is not allowed, all of them should really be positive
        bool p = this->m_isSecondPlayer && (opts["same-dual"]);
        short od = this->m_isSecondPlayer && (opts["sep-dual"]) ? 180 : 0;
        short o2 = opts["sep-second"] ? 120 : 0;
        short o3 = opts["sep-glow"] ? 240 : 0;
        // easy-mode
        short id = 0;
        // adv mode
        if (!opts["easy"]) {
            id = 1;
            if (this->m_isShip)
                id = Level && Level->isPlatformer() ? 9 : 2;
            else if (this->m_isBall)
                id = 3;
            else if (this->m_isBird)
                id = 4;
            else if (this->m_isDart)
                id = 5;
            else if (this->m_isRobot)
                id = 6;
            else if (this->m_isSpider)
                id = 7;
            else if (this->m_isSwing)
                id = 8;
        }

        // get the two chroma pattern result firstly
        ccColor3B main = getChroma(setups[getIndex(p, id, Channel::Main)],
            m_fields->main, lvlphase + od, percentage, reset[this]);
        ccColor3B secondary = getChroma(setups[getIndex(p, id, Channel::Secondary)],
            m_fields->secondary, lvlphase + od + o2, percentage, reset[this]);
        ccColor3B glow = getChroma(setups[getIndex(p, id, Channel::Glow)],
            m_glowColor, lvlphase + od + o3, percentage, reset[this]);

        // for compatibility with Seperate dual icons or other mods, here I should always avoid using PlayerObject's own member functions
        bool isRider = false;
        // icons
        if (m_isShip || m_isBird) {
            isRider = opts["rider"] && id;
            this->m_iconSprite->setColor(isRider ? getChroma(setups[getIndex(p,  1, Channel::Main)],
                m_fields->main, lvlphase + od, percentage, reset[this]) : main);
            this->m_iconSpriteSecondary->setColor(isRider ? getChroma(setups[getIndex(p, 1, Channel::Secondary)],
                m_fields->secondary, lvlphase + od + o2, percentage, reset[this]) : secondary);
            if (m_hasGlow)
                this->m_iconGlow->setColor(isRider ? getChroma(setups[getIndex(p, 1, Channel::Glow)],
                    m_glowColor, lvlphase + od + o3, percentage, reset[this]) : glow);

            // vehicles
            this->m_vehicleSprite->setColor(main);
            this->m_vehicleSpriteSecondary->setColor(secondary);
            if (m_hasGlow)
                this->m_vehicleGlow->setColor(glow);
        }
        // robot
        else if (m_isRobot) {
            auto arr = this->m_robotSprite->getChildByType<CCPartAnimSprite>(0)->m_spriteParts;
            for (auto part : CCArrayExt<CCSpritePart*>(arr)) {
                part->setColor(main);
                part->getChildByType<CCSprite>(0)->setColor(secondary);
            }
            if (m_hasGlow)
                for (auto spr: CCArrayExt<CCSprite*>(this->m_robotSprite->getChildByType<CCSprite>(0)->getChildren()))
                    spr->setColor(glow);
        }
        // spider
        else if (m_isSpider) {
            auto arr = this->m_spiderSprite->getChildByType<CCPartAnimSprite>(0)->m_spriteParts;
            for (auto part : CCArrayExt<CCSpritePart*>(arr)) {
                part->setColor(main);
                part->getChildByType<CCSprite>(0)->setColor(secondary);
            }
            if (m_hasGlow)
                for (auto spr: CCArrayExt<CCSprite*>(this->m_spiderSprite->getChildByType<CCSprite>(0)->getChildren()))
                    spr->setColor(glow);
        }
        // regular modes
        else {
            this->m_iconSprite->setColor(main);
            this->m_iconSpriteSecondary->setColor(secondary);
            if (m_hasGlow)
                this->m_iconGlow->setColor(glow);
        }

        // trail
        if (gm->getPlayerStreak() != 2)
            this->m_regularTrail->setColor(getChroma(setups[getIndex(p, 11, Channel::Effect)],
                m_fields->secondary, lvlphase + od, percentage, reset[this]));
        // wave trail
        if (this->m_isDart)
            this->m_waveTrail->setColor(getChroma(setups[getIndex(p, 12, Channel::Effect)],
                gm->getGameVariable("0096") ? m_fields->secondary : m_fields->main, lvlphase + od, percentage, reset[this]));
        // dash fire
        if (this->m_isDashing)
            this->m_dashFireSprite->setColor(getChroma(setups[getIndex(p, 13, Channel::Effect)],
                gm->getGameVariable("0062") ? m_fields->main : m_fields->secondary, lvlphase + od, percentage, reset[this]));
        // teleport line
        // ?
        // ufo shell
        if (this->m_isBird)
            this->m_birdVehicle->setColor(getChroma(setups[getIndex(p, 15, Channel::Effect)],
                ccc3(255, 255, 255), lvlphase + od, percentage, reset[this]));

        if (reset[this])
            reset[this] --;
    }

    void setColor(ccColor3B const &color) override {
        PlayerObject::setColor(color);
        m_fields->main = color;
    }
    void setSecondColor(ccColor3B const &color) {
        PlayerObject::setSecondColor(color);
        m_fields->secondary = color;
    }
    
    ~ChromaPlayer() {
        reset.erase(this);
    }
};

// control default values
$on_mod(Loaded) {
    // load setups
    for (short p = 0; p < 2; p++) {
        for (short id = 0; id < 10; id++)
            for (short chnl = 0; chnl < 3; chnl++)
                setups[getIndex(p, id, Channel(chnl))] = Mod::get()->getSavedValue<ChromaSetup>(getConfigKey(p, id, Channel(chnl)), DEFAULT_SETUP);
        for (short id = 11; id < 16; id++)
            setups[getIndex(p, id, Channel::Effect)] = Mod::get()->getSavedValue<ChromaSetup>(getConfigKey(p, id, Channel::Effect), DEFAULT_SETUP);
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
        {"sep-dual", false},
        {"sep-second", false},
        {"sep-glow", false},
        {"prev", true},
        {"blur-bg", false},
        {"dark-theme", true}
    };
    for (auto [key, val] : defaultOpts)
        opts[key] = Mod::get()->getSavedValue<bool>(key, val);
}