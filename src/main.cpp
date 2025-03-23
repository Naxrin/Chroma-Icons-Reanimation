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
// reset chroma
extern std::map<PlayerObject*, bool> reset;
// globed
static bool globed;
// timewarp
static float timewarp;
// bools
extern std::map<std::string, bool> opts;
// speed option
extern float speed;
// setup center
extern std::map<short, ChromaSetup> setups;

// main/second/glow/detail of globed progress bar icon
ccColor3B barm, bars, barg, barw;

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

// timewarp
#include <Geode/modify/GJBaseGameLayer.hpp>
class $modify(MyBaseGameLayer, GJBaseGameLayer) {
    void updateTimeWarp(float p) override {
        GJBaseGameLayer::updateTimeWarp(p);
        timewarp = p;
    }

    void resetPlayer() {
        GJBaseGameLayer::resetPlayer();
        timewarp = 1;
    }
};

// mainly global phase operation
#include <Geode/modify/PlayLayer.hpp>
class $modify(GameLayer, PlayLayer) {
    struct Fields {
        float is_globed = false;
    };
    // reset phase and tell mod central the level type in init
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        m_fields->is_globed = Loader::get()->isModLoaded("dankmeme.globed2") && !level->isPlatformer();
        // reset phase
        lvlphase = 0;
        // percentage = 0
        percentage = 0;
        // register level
        Level = level;
        // allow chroma
        layerType = LayerType::PlayLayer;
        reset.clear();
        // progress
        progress = level->m_normalPercent.value();
        return PlayLayer::init(level, useReplay, dontCreateObjects);
    }

    // edit phase
    void postUpdate(float d) {
        // iterate phase and update progress
        lvlphase = fmod(lvlphase + 360 * d * speed / (opts["igntw"] ? timewarp : 1), 360.f);
        percentage = this->getCurrentPercent();
        PlayLayer::postUpdate(d);
        // globed
        if (!opts["globed"])
            if (globed)
                globed = false;
            else
                return;
        if (!m_fields->is_globed || !m_progressBar->isVisible())
            return;
        auto node = m_progressBar->getChildByID("dankmeme.globed2/progress-bar-wrapper");
        if (!node)
            return;

        auto myself = node->getChildByID("dankmeme.globed2/self-player-progress");
        if (auto point = myself->getChildByType<CCLayerColor>(0))
            if (opts["globed"])
                point->setColor(barm);
        
        if (auto gplayer = static_cast<CCNode*>(myself->getChildren()->objectAtIndex(1)))
            if (auto player = static_cast<SimplePlayer*>(gplayer->getChildren()->objectAtIndex(0)))
                if (opts["globed"]) {
                    // main
                    player->setColor(barm);
                    // second
                    player->setSecondColor(bars);
                    // glow
                    if (player->m_hasGlowOutline)
                        player->setGlowOutline(barg);
                    // white
                    player->m_detailSprite->setColor(barw);
                }
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
        // get progress
        progress = level->m_normalPercent.value();
		return LevelEditorLayer::init(level, p);
    }

    // edit phase
    void postUpdate(float d) override {
        if (opts["editor"])
            lvlphase = fmod(lvlphase + 360 * d * speed / (opts["igntw"] ? timewarp : 1), 360.f);
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
            lvlphase = fmod(lvlphase + 360 * d * speed / (opts["igntw"] ? timewarp : 1), 360.f);

        MenuGameLayer::update(d);
    }

    void resetPlayer() {
        if (opts["???"])
            progress = std::rand() % 100;
        MenuGameLayer::resetPlayer();
    }
};

inline std::string toIndexStr(int index) {
    return (index < 10 ? "0" : "") + std::to_string(index);
}

#include <Geode/modify/GhostTrailEffect.hpp>
class $modify(OptionalGhostTrail, GhostTrailEffect) {
    // generate robtop's ghost trail only when my own proxy is off
    void trailSnapshot(float p) {
        if (!opts["activate"] || opts["dis-ghost"])
            GhostTrailEffect::trailSnapshot(p);
    }
};

// My god, this mod finally starts to chroma your icons as definitely expected from here on
#include <Geode/modify/PlayerObject.hpp>
class $modify(ChromaPlayer, PlayerObject) {
    struct Fields {
        // if the counter reaches the period of generator, the game should generate a ghost trail here and reset the counter
        float ghost_counter = 0;
        // default color
        ccColor3B main, second, glow;
        // frame name for ghost trail sprite name
        std::string frame, ball, dart, swing, robot, spider;
    };
    // record this player's pointer
    bool init(int p0, int p1, GJBaseGameLayer *p2, cocos2d::CCLayer *p3, bool p4) {
        reset[this] = false;
        // get sprite names for ghost trails generation
        this->getTextureNames();
        return PlayerObject::init(p0, p1, p2, p3, p4);
    }

    // compact with separate dual icons and more icons
    void getTextureNames() {
        auto MI = Loader::get()->getLoadedMod("hiimjustin000.more_icons");
        auto SDI = Loader::get()->getLoadedMod("weebify.separate_dual_icons");
        bool usingSDI = SDI && m_isSecondPlayer;
        // frame
        if (MI) {
            auto name = MI->getSavedValue<std::string>(usingSDI ? "icon-dual" : "icon");
            if (name != "")
                m_fields->frame = fmt::format("hiimjustin000.more_icons/{}_001.png", name);
            else
                m_fields->frame = fmt::format("player_{}_001.png", toIndexStr(usingSDI ? SDI->getSavedValue<int64_t>("cube") : gm->getPlayerFrame()));
        } else
            m_fields->frame = fmt::format("player_{}_001.png", toIndexStr(usingSDI ? SDI->getSavedValue<int64_t>("cube") : gm->getPlayerFrame()));

        // ball
        if (MI) {
            auto name = MI->getSavedValue<std::string>(usingSDI ? "ball-dual" : "ball");
            if (name != "")
                m_fields->ball = fmt::format("hiimjustin000.more_icons/{}_001.png", name);
            else
                m_fields->ball = fmt::format("player_ball_{}_001.png", toIndexStr(usingSDI ? SDI->getSavedValue<int64_t>("roll") : gm->getPlayerBall()));
        } else
            m_fields->ball = fmt::format("player_ball_{}_001.png", toIndexStr(usingSDI ? SDI->getSavedValue<int64_t>("roll") : gm->getPlayerBall()));

        // wave
        if (MI) {
            auto name = MI->getSavedValue<std::string>(usingSDI ? "wave-dual" : "wave");
            if (name != "")
                m_fields->dart = fmt::format("hiimjustin000.more_icons/{}_001.png", name);
            else
                m_fields->dart = fmt::format("dart_{}_001.png", toIndexStr(usingSDI ? SDI->getSavedValue<int64_t>("dart") : gm->getPlayerDart()));
        } else
            m_fields->dart = fmt::format("dart_{}_001.png", toIndexStr(usingSDI ? SDI->getSavedValue<int64_t>("dart") : gm->getPlayerDart()));

        // swing
        if (MI) {
            auto name = MI->getSavedValue<std::string>(usingSDI ? "swing-dual" : "swing");
            if (name != "")
                m_fields->swing = fmt::format("hiimjustin000.more_icons/{}_001.png", name);
            else
                m_fields->swing = fmt::format("swing_{}_001.png", toIndexStr(usingSDI ? SDI->getSavedValue<int64_t>("swing") : gm->getPlayerSwing()));
        } else
            m_fields->swing = fmt::format("swing_{}_001.png", toIndexStr(usingSDI ? SDI->getSavedValue<int64_t>("swing") : gm->getPlayerSwing()));

        // robot
        if (MI) {
            auto name = MI->getSavedValue<std::string>(usingSDI ? "robot-dual" : "robot");
            if (name != "")
                m_fields->robot = fmt::format("hiimjustin000.more_icons/{}_001.png", name);
            else
                m_fields->robot = fmt::format("robot_{}_01_001.png", toIndexStr(usingSDI ? SDI->getSavedValue<int64_t>("robot") : gm->getPlayerRobot()));
        } else
            m_fields->robot = fmt::format("robot_{}_01_001.png", toIndexStr(usingSDI ? SDI->getSavedValue<int64_t>("robot") : gm->getPlayerRobot()));

        // spider
        if (MI) {
            auto name = MI->getSavedValue<std::string>(usingSDI ? "spider-dual" : "spider");
            if (name != "")
                m_fields->spider = fmt::format("hiimjustin000.more_icons/{}_001.png", name);
            else
                m_fields->spider = fmt::format("spider_{}_01_001.png", toIndexStr(usingSDI ? SDI->getSavedValue<int64_t>("spider") : gm->getPlayerSpider()));
        } else
            m_fields->spider = fmt::format("spider_{}_01_001.png", toIndexStr(usingSDI ? SDI->getSavedValue<int64_t>("spider") : gm->getPlayerSpider()));
        
        // colors
        m_fields->main = gm->colorForIdx(usingSDI ? SDI->getSavedValue<int64_t>("color1") : gm->getPlayerColor());
        m_fields->second = gm->colorForIdx(usingSDI ? SDI->getSavedValue<int64_t>("color2") : gm->getPlayerColor2());
        m_fields->glow = gm->colorForIdx(usingSDI ? SDI->getSavedValue<int64_t>("colorglow") : gm->getPlayerGlowColor());
    }

    // update override, chroma icons regarding current setup
    void update(float d) override {
        // ghost trail proxy
        m_fields->ghost_counter += d;
        if (m_fields->ghost_counter > 3) {
            // enabled ghost and using my proxy
            if (m_ghostType == GhostType::Enabled && opts["activate"] && !opts["dis-ghost"])
                this->generateChromaGhostTrail();
            m_fields->ghost_counter = fmod(m_fields->ghost_counter, 3);
        }
        // base
        this->PlayerObject::update(d);
        // run chroma of other sprites
        if (!opts["init"])
            this->processChroma();
    }
    
    // set position override covers level pause in layer init 
    void setPosition(CCPoint const& pos) override {
        PlayerObject::setPosition(pos);
        if (opts["init"])
            // set chroma color
            this->processChroma();  
    }

    // get some phase offsets then set chroma color
    // as sending negative phase value to getChroma(...) is not allowed, all of them should really be positive
    void processChroma() {
        // only chroma the visible two
        if ((!opts["activate"] && !reset[this]) || !this->isVisible() || this->getTag() > 0)
            return;
        if (layerType == LayerType::LevelEditorLayer && !opts["editor"])
            return;
        if (layerType == LayerType::MenuLayer) {
            if (opts["???"])
                percentage = 100 * this->getPositionX() / CCDirector::sharedDirector()->getWinSize().width;
            else
                return;
        }

        bool p = this->m_isSecondPlayer && !opts["same-dual"];
        short od = this->m_isSecondPlayer && (opts["sep-dual"]) ? 180 : 0;
        short o2 = opts["sep-second"] ? 120 : 0;
        short o3 = opts["sep-glow"] ? 240 : 0;
        // easy-mode
        Gamemode status = this->getStatusID();

        // get the chroma pattern result firstly
        ccColor3B main = getChroma(setups[getIndex(p, status, Channel::Main)], m_fields->main, lvlphase + od, percentage, progress);
        ccColor3B secondary = getChroma(setups[getIndex(p, status, Channel::Secondary)], m_fields->second, lvlphase + od + o2, percentage, progress);
        ccColor3B glow = getChroma(setups[getIndex(p, status, Channel::Glow)], m_fields->glow, lvlphase + od + o3, percentage, progress);
        ccColor3B white = getChroma(setups[getIndex(p, status, Channel::White)], ccc3(255, 255, 255), lvlphase + od, percentage, progress);

        if (!this->m_isSecondPlayer) {
            if (opts["globed"]) {
                // recolor
                barm = main; bars = secondary; barg = glow; barw = white;
                // set value
                globed = true;
            } else if (globed) {
                // recover
                barm = m_fields->main; bars = m_fields->second; barg = m_fields->glow; barw = ccc3(255, 255, 255);
            }
        }


        // icons
        // for compatibility with Seperate dual icons or other mods
        // here I should always avoid calling their member functions
        bool isRider = false;

        // icons with vehicles
        if (m_isShip || m_isBird) {
            isRider = opts["rider"] && (int)status;
            // icon
            this->m_iconSprite->setColor(isRider ? getChroma(setups[getIndex(p, Gamemode::Cube, Channel::Main)],
                m_fields->main, lvlphase + od, percentage, progress) : main);
            this->m_iconSpriteSecondary->setColor(isRider ? getChroma(setups[getIndex(p, Gamemode::Cube, Channel::Secondary)],
                m_fields->second, lvlphase + od + o2, percentage, progress) : secondary);
            if (m_hasGlow)
                this->m_iconGlow->setColor(isRider ? getChroma(setups[getIndex(p, Gamemode::Cube, Channel::Glow)],
                    m_fields->glow, lvlphase + od + o3, percentage, progress) : glow);
            if (this->m_iconSpriteWhitener)
                this->m_iconSpriteWhitener->setColor(isRider ? getChroma(setups[getIndex(p, Gamemode::Cube, Channel::White)],
                    ccc3(255, 255, 255), lvlphase + od, percentage, progress) : white);
            // vehicle
            this->m_vehicleSprite->setColor(main);
            this->m_vehicleSpriteSecondary->setColor(secondary);
            if (m_hasGlow)
                this->m_vehicleGlow->setColor(glow);
            if (this->m_vehicleSpriteWhitener)
                this->m_vehicleSpriteWhitener->setColor(white);
        }
        // robot
        else if (m_isRobot) {
            this->m_robotSprite->m_color = main;
            this->m_robotSprite->m_secondColor = secondary;
            this->m_robotSprite->updateColors();
            if (m_hasGlow)
                for (auto spr: CCArrayExt<CCSprite*>(this->m_robotSprite->m_glowSprite->getChildren()))
                    spr->setColor(glow);
            this->m_robotSprite->m_extraSprite->setColor(white);
        }
        // spider
        else if (m_isSpider) {
            this->m_spiderSprite->m_color = main;
            this->m_spiderSprite->m_secondColor = secondary;
            this->m_spiderSprite->updateColors();
            if (m_hasGlow)
                for (auto spr: CCArrayExt<CCSprite*>(this->m_spiderSprite->m_glowSprite->getChildren()))
                    spr->setColor(glow);
            this->m_spiderSprite->m_extraSprite->setColor(white);
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
            this->m_regularTrail->setColor(getChroma(setups[getIndex(p, status, Channel::Trail)],
                m_fields->second, lvlphase + od, percentage, progress));
        // wave trail
        if (this->m_isDart)
            this->m_waveTrail->setColor(getChroma(setups[getIndex(p, status, Channel::WaveTrail)],
                gm->getGameVariable("0096") ? m_fields->second : m_fields->main, lvlphase + od, percentage, progress));
        // dash fire
        if (this->m_isDashing)
            this->m_dashFireSprite->setColor(getChroma(setups[getIndex(p, status, Channel::DashFire)],
                gm->getGameVariable("0062") ? m_fields->main : m_fields->second, lvlphase + od, percentage, progress));

        // ufo shell
        if (this->m_isBird)
            this->m_birdVehicle->setColor(getChroma(setups[getIndex(p, status, Channel::UFOShell)],
                ccc3(255, 255, 255), lvlphase + od, percentage, progress));                

        // confirm it's really reset
        reset[this] = false;
    }

    // rewrite ghost trail generator
    void generateChromaGhostTrail() {
        // some arguments
        auto p = this->getPosition();
        auto s = this->getScale();
        auto r = this->getRotation() + (this->m_isSideways ? 90.f : 0.f);

        // name
        std::string name;
        if (m_isRobot) {
            auto main = m_robotSprite->getChildByType<CCPartAnimSprite>(0)->getChildByTag(1);
            auto dp = main->getPosition();
            auto dr = main->getRotation();
            p.x += dp.x;
            p.y += dp.y;
            r += dr;
            name = m_fields->robot;
        }
        else if (m_isSpider) {
            auto main = m_spiderSprite->getChildByType<CCPartAnimSprite>(0)->getChildByTag(1);
            auto dp = main->getPosition();
            auto dr = main->getRotation();
            p.x += dp.x;
            p.y += dp.y;
            r += dr;
            name = m_fields->spider;
        }
        else if (m_isShip || m_isBird) {
            bool isJetpack = m_isShip && m_isPlatformer;
            p.x += isJetpack ? 6.f : 0.f;
            p.y += isJetpack ? 4.f : 5.f;
            s = s * (isJetpack ? 0.6 : 0.55);
            name = m_fields->frame;
        } else if (m_isBall)
            name = m_fields->ball;
        else if (m_isDart)
            name = m_fields->dart;
        else if (m_isSwing)
            name = m_fields->swing;
        else
            name = m_fields->frame;

        // generate
        auto spr = CCSprite::createWithSpriteFrameName(name.c_str());
        spr->setPosition(p);
        spr->setScale(s);
        spr->setRotation(r);
        spr->setFlipX(this->m_isGoingLeft != this->m_isSideways);
        spr->setFlipY(this->m_isUpsideDown);

        // this mod is chroma icons not ghost trail fix        
        auto color = getChroma(setups[getIndex(this->m_isSecondPlayer && !opts["same-dual"], this->getStatusID(), Channel::Ghost)],
            m_fields->main, lvlphase + ((this->m_isSecondPlayer && opts["sep-dual"]) ? 180.f : 0), percentage, progress);
        spr->setColor(color);
        // this is how robtop set his own ghost trails
        if (color == ccc3(0, 0, 0))
            spr->setBlendFunc({GL_ONE, GL_ONE_MINUS_SRC_ALPHA});
        else
            spr->setBlendFunc({GL_SRC_ALPHA, GL_ONE});

        // add to and run action
        this->getParent()->addChild(spr);
        spr->runAction(CCEaseOut::create(CCScaleTo::create(0.5, 0.6), 2));
        spr->runAction(CCFadeOut::create(0.5));
        spr->runAction(CCSequence::create(
            CCDelayTime::create(0.5),
            CallFuncExt::create([spr]() { spr->removeFromParentAndCleanup(true); }),
            nullptr
        ));
    }

    void playSpiderDashEffect(CCPoint from, CCPoint to) {
        PlayerObject::playSpiderDashEffect(from, to);
        if (!opts["activate"])
            return;
        
        for (auto node : CCArrayExt<CCNode*>(this->getParent()->getChildren())) {
            // stupid judge
            if (auto tele = typeinfo_cast<CCSprite*>(node)) {
                // check class type and content size
                if (typeinfo_cast<PlayerObject*>(node) || !(tele->getContentSize() == CCSize(221.f, 21.f) ))
                    continue;
                // check position
                if (detectTPline(tele, from, to))
                    tele->setColor(getChroma(setups[getIndex(this->m_isSecondPlayer && !opts["same-dual"], this->getStatusID(), Channel::TPLine)],
                        m_fields->main, lvlphase + ((this->m_isSecondPlayer && opts["sep-dual"]) ? 180.f : 0), percentage, progress));
            }
        }
    }
    
    bool detectTPline(CCSprite *tele, CCPoint &from, CCPoint &to) {
        CCPoint pos = tele->getPosition();
        CCPoint posfix;
        // horizental tp
        if (this->m_isSideways) {
            posfix = CCPoint((from.x + to.x) / 2, from.y);
            //log::error("delta x = {} y = {}", pos.x - posfix.x + (this->m_isGoingLeft ? 4.5 : -10.5), pos.y - posfix.y);
            if (std::abs(pos.y - posfix.y) > 0.001 || std::abs(pos.x - posfix.x + (this->m_isGoingLeft ? 4.5 : -10.5)) > 0.001
                || tele->getRotation() != this->m_isUpsideDown * 180.f)
                return false;
        }
        // vertical tp
        else {
            posfix = CCPoint(from.x, (from.y + to.y) / 2);
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

    Gamemode getStatusID() {
        if (opts["easy"]) return Gamemode::Icon;
        if (this->m_isShip) return Level && Level->isPlatformer() ? Gamemode::Jetpack : Gamemode::Ship;
        else if (this->m_isBall) return Gamemode::Ball;
        else if (this->m_isBird) return Gamemode::Ufo;
        else if (this->m_isDart) return Gamemode::Wave;
        else if (this->m_isRobot) return Gamemode::Robot;
        else if (this->m_isSpider) return Gamemode::Spider;
        else if (this->m_isSwing) return Gamemode::Swing;
        return Gamemode::Cube;
    }
};

// control default values
$on_mod(Loaded) {
    // load setups
    for (short p = 0; p < 2; p++) {
        // icons
        for (short gmid = 0; gmid < 10; gmid++)
            for (short chnl = 0; chnl < 8; chnl++)
                setups[getIndex(p, Gamemode(gmid), Channel(chnl))] = Mod::get()->getSavedValue<ChromaSetup>(getConfigKey(p, Gamemode(gmid), Channel(chnl)), DEFAULT_SETUP);
        // wave trail
        setups[getIndex(p, Gamemode::Icon, Channel::WaveTrail)] = Mod::get()->getSavedValue<ChromaSetup>(getConfigKey(p, Gamemode::Icon, Channel::WaveTrail), DEFAULT_SETUP);
        setups[getIndex(p, Gamemode::Wave, Channel::WaveTrail)] = Mod::get()->getSavedValue<ChromaSetup>(getConfigKey(p, Gamemode::Wave, Channel::WaveTrail), DEFAULT_SETUP);
        // ufo shell
        setups[getIndex(p, Gamemode::Icon, Channel::UFOShell)] = Mod::get()->getSavedValue<ChromaSetup>(getConfigKey(p, Gamemode::Icon, Channel::UFOShell), DEFAULT_SETUP);
        setups[getIndex(p, Gamemode::Ufo, Channel::UFOShell)] = Mod::get()->getSavedValue<ChromaSetup>(getConfigKey(p, Gamemode::Ufo, Channel::UFOShell), DEFAULT_SETUP);
    }
    // speed
    speed = Mod::get()->getSavedValue<float>("speed", 1);
    // load options
    std::map<std::string, bool> defaultOpts = {
        {"easy", true},
        {"activate", true},
        {"same-dual", false},
        {"rider", false},
        {"igntw", false},
        {"globed", false},
        {"editor", false},
        {"init", true},
        {"dis-ghost", false},
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
