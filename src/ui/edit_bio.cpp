#include <Geode/Geode.hpp>
#include <UIBuilder.hpp>
#include <regex>

#include "ui/edit_bio.hpp"

using namespace geode::prelude;

EditBioPopup* EditBioPopup::create(ProfileData* const& profile_data) {
    auto ret = new EditBioPopup();
    if (ret && ret->initAnchored(450.f, 280.f, profile_data)) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool EditBioPopup::setup(ProfileData* const& profile_data) {
    this->m_profile_data = profile_data;
    log::info("current bio: {}", profile_data->bio);
    auto win_size = CCDirector::sharedDirector()->getWinSize();

    this->setTitle("Change Bio (WIP!)");

    Build<CCLabelBMFont>::create("newlines are very broken sadly (blame rob)", "goldFont.fnt")
        .id("newlines-brokey-1"_spr)
        .scale(0.5f)
        .pos(win_size / 2 + ccp(0.f, 105.f))
        .parent(m_mainLayer)
        .intoNewSibling(CCLabelBMFont::create("<nl> will get replaced with a newline", "goldFont.fnt"))
            .id("newlines-brokey-2"_spr)
            .scale(0.5f)
            .pos(win_size / 2 + ccp(0.f, 95.f))
            .intoNewSibling(CCLabelBMFont::create("editing with newlines will be buggy!", "goldFont.fnt"))
                .id("newlines-brokey-3"_spr)
                .scale(0.5f)
                .pos(win_size / 2 + ccp(0.f, 85.f));

    // this bitch does NOT support newlines :(
    Build<CCTextInputNode>::create(400.f, 200.f, "bio placeholder bleh !!", "chatFont.fnt")
        .id("bio-text-field"_spr)
        .pos(win_size / 2)
        .store(m_bio_text_node)
        .parent(m_mainLayer);

    Build<ButtonSprite>::create("Preview", 64, true, "bigFont.fnt", "GJ_button_01.png", 32.0f, 1.0f)
            .intoMenuItem([this](auto) {
                // todo: make this a custom popup
                geode::MDPopup::create("Preview", m_bio_text_node->getString(), "useless", "button", [](bool a){
                    // very cool
                })->show();
            })
                //.store(m_login_button)
                .id("preview-button"_spr)
                .pos(0.f, -110.f)
                .parent(m_buttonMenu);

    // idfk - todo: make class based on CCTextInputNode that supports newlines (and doesn't break when using them)
    m_bio_text_node->setAllowedChars("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz,;.:-_<>^°1!2\"34$5%6&7/8(9)0=?´+*#'\n[] ");
    m_bio_text_node->setString(profile_data->bio.value_or("").c_str());
    m_bio_text_node->setDelegate(this);

    return true;
}

void EditBioPopup::onClose(cocos2d::CCObject* a) {
    // store text from popup when user closes the popup (to not discard what they wrote)
    auto str = m_bio_text_node->getString();
    if (str.empty()) {
        m_profile_data->bio = std::nullopt;
    } else {
        this->m_profile_data->bio = str;
    }

    geode::Popup<ProfileData* const&>::onClose(a);
}

void EditBioPopup::textChanged(CCTextInputNode* a) {
    // replace <nl> with newline
    // not <br> since thats reserved for markdown stuff
    std::string text = a->getString();
    std::string new_text = std::regex_replace(text, std::regex("<nl>"), "\n");

    if (text != new_text) {
        a->setString(new_text.c_str());
    } else {
        TextInputDelegate::textChanged(a);
    }
}
