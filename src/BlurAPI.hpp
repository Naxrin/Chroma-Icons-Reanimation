#pragma once

#include <Geode/Geode.hpp>

#define BLUR_TAG "thesillydoggo.blur-api/blur-options"

namespace BlurAPI
{
    class BlurOptions : public cocos2d::CCObject
    {
        public:
            int apiVersion = 1; // dont change
            cocos2d::CCRenderTexture* rTex = nullptr;
            geode::Ref<cocos2d::CCClippingNode> clip = nullptr;
            bool forcePasses = false;
            int passes = 3;
            float alphaThreshold = 0.01f;
            
            bool needsMacOSWorkaround = false;
            cocos2d::CCSize lastContentSize = cocos2d::CCSizeZero;
            float lastScale = 1.0f;

            virtual bool init() { 
                #ifdef GEODE_IS_MACOS
                needsMacOSWorkaround = true;
                #endif
                return true; 
            }
            
            CREATE_FUNC(BlurOptions);
    };

    inline BlurOptions* getOptions(cocos2d::CCNode* node)
    {
        return static_cast<BlurOptions*>(node->getUserObject(BLUR_TAG));
    }

    inline void addBlur(cocos2d::CCNode* node)
    {
        if (getOptions(node))
            return;

        auto options = BlurOptions::create();
        
        #ifdef GEODE_IS_MACOS
        if (node->getContentSize().width > 0 && node->getContentSize().height > 0) {
            options->rTex = cocos2d::CCRenderTexture::create(
                static_cast<int>(node->getContentSize().width),
                static_cast<int>(node->getContentSize().height),
                cocos2d::kTexture2DPixelFormat_RGBA8888
            );
            options->rTex->retain();
        }
        #endif
        
        node->setUserObject(BLUR_TAG, options);
    }

    inline void removeBlur(cocos2d::CCNode* node)
    {
        auto options = getOptions(node);
        if (options) {
            #ifdef GEODE_IS_MACOS
            if (options->rTex) {
                options->rTex->release();
                options->rTex = nullptr;
            }
            #endif
        }
        node->setUserObject(BLUR_TAG, nullptr);
    }

    inline bool isBlurAPIEnabled()
    {
        if (auto blur = geode::Loader::get()->getLoadedMod("thesillydoggo.blur-api"))
        {
            if (blur->getSettingValue<bool>("enabled"))
                return true;
        }

        return false;
    }

    inline bool willLoad()
    {
        if (auto blur = geode::Loader::get()->getInstalledMod("thesillydoggo.blur-api"))
        {
            if (blur->shouldLoad())
            {
                return true;
            }
        }

        return false;
    }

    inline void updateRenderTextureForMacOS(cocos2d::CCNode* node, BlurOptions* options) {
        #ifdef GEODE_IS_MACOS
        if (!options || !node) return;
        
        auto contentSize = node->getContentSize();
        auto scale = node->getScale();
        
        bool needsUpdate = false;
        
        if (!options->rTex) {
            needsUpdate = true;
        } else if (contentSize.width != options->lastContentSize.width || 
                   contentSize.height != options->lastContentSize.height) {
            needsUpdate = true;
        } else if (scale != options->lastScale) {
            needsUpdate = true;
        }
        
        if (needsUpdate && contentSize.width > 0 && contentSize.height > 0) {
            if (options->rTex) {
                options->rTex->release();
            }
            
            int texWidth = static_cast<int>(contentSize.width * scale);
            int texHeight = static_cast<int>(contentSize.height * scale);
            
            texWidth = std::max(1, texWidth);
            texHeight = std::max(1, texHeight);
            
            options->rTex = cocos2d::CCRenderTexture::create(
                texWidth, texHeight,
                cocos2d::kTexture2DPixelFormat_RGBA8888
            );
            options->rTex->retain();
            
            options->lastContentSize = contentSize;
            options->lastScale = scale;
        }
        #endif
    }
};