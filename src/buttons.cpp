#include <Geode/modify/GJGarageLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/loader/Mod.hpp>

#include "layer.hpp"

using namespace geode::prelude;

class $modify(IconLayer, GJGarageLayer){
	bool init(){
		if (!GJGarageLayer::init()) return false;

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

	void onChromaMenu(CCObject *sender){
		ChromaLayer::create()->show();
	}
};

class $modify(NivelEditorLayer, LevelEditorLayer){
	bool init(GJGameLevel *level, bool p){
		if (!LevelEditorLayer::init(level, p)) return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto menu = m_editorUI->getChildByID("undo-menu");

        if (menu && Mod::get()->getSavedValue<bool>("editor", true)){
            // add button
            auto img = CCSprite::create("rgbicon.png"_spr);
            img->setScale(0.6);

            auto button = CCMenuItemSpriteExtra::create(img, this, menu_selector(NivelEditorLayer::onChromaMenu));
            button->setID("icon-chroma-button");
            button->setPosition(CCPoint(163.f, 19.75));

            menu->addChild(button);
            menu->updateLayout();
        }
		return true;
	}

	void onChromaMenu(CCObject *sender){
		ChromaLayer::create()->show();
	}
};

class $modify(PoseLayer, PauseLayer){
    
	static void onModify(auto& self) {
        Result<> plCreate = self.setHookPriority("PauseLayer::create", -99); // coins in pause menu first
    }

	void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        //button
        auto img = CCSprite::create("rgbicon.png"_spr);
        img->setScale(0.6);
        auto button = CCMenuItemSpriteExtra::create(img, this, menu_selector(PoseLayer::onChromaMenu));//menu_selector(IconLayer::onColorButton)
        button->setID("icon-chroma-button");
        
        if (auto target = this->getChildByID("right-button-menu")) {
            target->addChild(button);
            target->updateLayout();
        }
	}

	void onChromaMenu(CCObject *){
        ChromaLayer::create()->show();
	}
};
