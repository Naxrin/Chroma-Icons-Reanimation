#pragma once

#include "utility.hpp"
// Geode's MD Text Area but no scroll layer
class FixedMDTextArea :
    public CCLayer,
    public CCLabelProtocol,
    public FLAlertLayerProtocol {
private:
    static std::string translateNewlines(std::string const& str);

    bool init(std::string str, cocos2d::CCSize const& size);
protected:
    FixedMDTextArea();
    virtual ~FixedMDTextArea();
private:
    void onLink(CCObject*);
    void onGDProfile(CCObject*);
    void onGDLevel(CCObject*);
    void onGeodeMod(CCObject*);
    void FLAlert_Clicked(FLAlertLayer*, bool btn) override;

    friend struct ::MDParser;
public:
    /**
     * Create a markdown text area. See class
     * documentation for details on supported
     * features & notes.
     * @param str String to render
     * @param size Size of the textarea
     */
    static FixedMDTextArea* create(std::string str, cocos2d::CCSize const& size);

    /**
     * Create a markdown text area. See class
     * documentation for details on supported
     * features & notes.
     * @param str String to render
     * @param size Size of the textarea
     * @param compatibilityMode Enables functionality that may be useful for wrapping a generic alert, such as newline support
     */
    static FixedMDTextArea* create(std::string str, cocos2d::CCSize const& size, bool compatibilityMode);

    /**
     * Update the label's content; call
     * sparingly as rendering may be slow
     */
    void updateLabel();

    void setString(char const* text) override;
    char const* getString() override;
};