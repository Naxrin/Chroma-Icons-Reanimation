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
extern std::map<PlayerObject*, bool> reset;
// bools
extern std::map<std::string, bool> opts;
// speed option
extern float speed;
// setup center
extern std::map<short, ChromaSetup> setups;


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
            button->setID("icon-chroma-button");
            
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

#include <Geode/modify/PauseLayer.hpp>
class $modify(PoseLayer, PauseLayer) {
	void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        // button
        auto img = CCSprite::create("rgbicon.png"_spr);
        img->setScale(0.6);
        auto button = CCMenuItemSpriteExtra::create(img, this, menu_selector(PoseLayer::onChromaMenu));//menu_selector(IconLayer::onColorButton)
        button->setID("icon-chroma-button");
        
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

#include <Geode/modify/PlayLayer.hpp>
class $modify(GameLayer, PlayLayer) {
    // reset phase and tell mod central the level type in init
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        // reset phase
        lvlphase = 0;
        // register level
        Level = level;
        // allow chroma
        allow_chroma = true;
        return PlayLayer::init(level, useReplay, dontCreateObjects);
    }
    // edit phase
    void postUpdate(float d) override {
        //log::warn("play layer update called");
        lvlphase = fmod(lvlphase + 360 * d * speed, 360.f);
        percentage = this->getCurrentPercent();
        PlayLayer::postUpdate(d);

        if (int c = m_unk37c0->count()) {
            for (int i = 1; i < c; i++)
                static_cast<CCNode*>(m_unk37c0->objectAtIndex(i))->setID("m_unk37a8");
        }
    }

    ~GameLayer() {
        allow_chroma = false;
    }
};

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
		if (!LevelEditorLayer::init(level, p))
            return false;

        // add button ony when editor option enabled
        auto menu = m_editorUI->getChildByID("undo-menu");
        if (!menu || !Mod::get()->getSavedValue<bool>("editor", false))
            return true;

        // add button
        auto img = CCSprite::create("rgbicon.png"_spr);
        img->setScale(0.6);

        auto button = CCMenuItemSpriteExtra::create(img, this, menu_selector(NivelEditorLayer::onChromaMenu));
        button->setID("icon-chroma-button");
        button->setPosition(CCPoint(163.f, 19.75));

        menu->addChild(button);
        menu->updateLayout();
        
		return true;
	}

	void onChromaMenu(CCObject *sender) {
		ChromaLayer::create()->show();
	}

    // edit phase
    void postUpdate(float d) override {
        if (opts["editor"])
            lvlphase = fmod(lvlphase + 360 * d * speed, 360.f);
        LevelEditorLayer::postUpdate(d);
    }

    ~NivelEditorLayer() {
        allow_chroma = false;
    }
};

#include <Geode/modify/PlayerObject.hpp>
class $modify(ChromaPlayer, PlayerObject) {
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
        bool p = this->m_isSecondPlayer && (opts["same-dual"]);
        short od = this->m_isSecondPlayer && (opts["sep-dual"]) ? 180 : 0;
        short o2 = opts["sep-second"] ? 120 : 0;
        short o3 = opts["sep-glow"] ? 120 : 0;
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

        // player
        this->setColor(getChroma(setups[getIndex(p, id, Channel::Main)],
            gm->colorForIdx(this->m_isSecondPlayer ? gm->getPlayerColor2() : gm->getPlayerColor()), lvlphase + od, percentage, reset[this]));
        this->setSecondColor(getChroma(setups[getIndex(p, id, Channel::Secondary)],
            gm->colorForIdx(this->m_isSecondPlayer ? gm->getPlayerColor() : gm->getPlayerColor2()), lvlphase + od + o2, percentage, reset[this]));
        if (this->m_hasGlow)
            this->setGlowColor(getChroma(setups[getIndex(p, id, Channel::Glow)],
                gm->colorForIdx(gm->getPlayerGlowColor()), lvlphase + od - o3, percentage, reset[this]));

        // vehicle
        if ((opts["rider"]) && (id == 2 || id == 4 || id == 9)) {
            this->m_iconSprite->setColor(getChroma(setups[getIndex(p, 1, Channel::Main)],
                gm->colorForIdx(this->m_isSecondPlayer ? gm->getPlayerColor2() : gm->getPlayerColor()), lvlphase + od, percentage, reset[this]));
            this->m_iconSpriteSecondary->setColor(getChroma(setups[getIndex(p, 1, Channel::Secondary)],
                gm->colorForIdx(this->m_isSecondPlayer ? gm->getPlayerColor() : gm->getPlayerColor2()), lvlphase + od + o2, percentage, reset[this]));
            if (this->m_hasGlow)
                this->m_iconGlow->setColor(getChroma(setups[getIndex(p, 1, Channel::Glow)],
                    gm->colorForIdx(gm->getPlayerGlowColor()), lvlphase + od - o3, percentage, reset[this]));
        }

        // trail
        if (gm->getPlayerStreak() != 2)
            this->m_regularTrail->setColor(getChroma(setups[getIndex(p, 11, Channel::Effect)],
                gm->colorForIdx(this->m_isSecondPlayer ? gm->getPlayerColor() : gm->getPlayerColor2()), lvlphase + od, percentage, reset[this]));
        // wave trail
        if (this->m_isDart)
            this->m_waveTrail->setColor(getChroma(setups[getIndex(p, 12, Channel::Effect)],
                gm->colorForIdx(this->m_isSecondPlayer == gm->getGameVariable("0096") ? gm->getPlayerColor() : gm->getPlayerColor2()), lvlphase + od, percentage, reset[this]));
        // dash fire
        if (this->m_isDashing)
            this->m_dashFireSprite->setColor(getChroma(setups[getIndex(p, 13, Channel::Effect)],
                gm->colorForIdx(this->m_isSecondPlayer == gm->getGameVariable("0062") ? gm->getPlayerColor2() : gm->getPlayerColor()), lvlphase + od, percentage, reset[this]));
        // teleport line
        // ?
        // ufo shell
        if (this->m_isBird)
            this->m_birdVehicle->setColor(getChroma(setups[getIndex(p, 15, Channel::Effect)],
                ccc3(255, 255, 255), lvlphase + od, percentage, reset[this]));

        reset[this] = false;                
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
            for (short chnl = 0; chnl < 2; chnl++)
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