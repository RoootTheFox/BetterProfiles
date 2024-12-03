#include <Geode/Geode.hpp>
#include <Geode/ui/ColorPickPopup.hpp>
#include <UIBuilder.hpp>

#include "ui/edit_background.hpp"
#include "utils.hpp"

using namespace geode::prelude;

EditBackgroundPopup* EditBackgroundPopup::create(ProfileData* const& profile_data) {
    auto ret = new EditBackgroundPopup();
    if (ret && ret->initAnchored(441.f, 291.5f, profile_data)) {
        ret->m_profile_data = profile_data;
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool EditBackgroundPopup::setup(ProfileData* const& profile_data) {
    auto win_size = CCDirector::sharedDirector()->getWinSize();

    this->setTitle("Customize Profile Gradient");

    // copy pasted from extended_profile.cpp (lazy!!)
    auto bg_sprite = this->m_mainLayer->getChildByType<CCScale9Sprite>(0);
    auto popup_content_size = CCSize(0.f, 0.f);
    if (bg_sprite) {
        popup_content_size = bg_sprite->getContentSize();
        bg_sprite->setVisible(false);

        auto col1 = Utils::intToColor(profile_data->color1.value_or(0));
        auto col2 = Utils::intToColor(profile_data->color2.value_or(0));

        Build<CCLayerGradient>::create(ccc4(col1.r, col1.g, col1.b, 255), ccc4(col2.r, col2.g, col2.b, 255))
            .contentSize(popup_content_size - ccp(5.f, 5.f))
            .ignoreAnchorPointForPos(false)
            .pos(win_size / 2)
            .parent(this->m_mainLayer)
            .store(this->m_background);

        Build<CCScale9Sprite>::create("GJ_square07.png")
            .contentSize(popup_content_size)
            .pos(win_size / 2)
            .parent(this->m_mainLayer);

        // when we're done with bg_sprite, nuke it
        bg_sprite->removeFromParentAndCleanup(true);
    } else {
        log::error("background is null");
    }

    Build<CCScale9Sprite>::create("square02b_001.png")
        .contentSize(popup_content_size - ccp(15.f, 15.f))
        .pos(win_size / 2)
        .color(ccc3(0, 0, 0))
        .opacity(50)
        .parent(this->m_mainLayer);
    // end copy paste meow

    auto main_menu = CCMenu::create();

    Build<ButtonSprite>::create("set end color", 122, true, "bigFont.fnt", "GJ_button_01.png", 32.0f, 1.0f)
        .intoMenuItem([this](auto) {
            this->m_editing_color = 2;
            auto popup = geode::ColorPickPopup::create(Utils::intToColor(this->m_profile_data->color2.value_or(0)));
            popup->setDelegate(this);
            popup->show();
        })
        .id("button-color2"_spr)
        .parent(main_menu);

    Build<ButtonSprite>::create("set start color", 122, true, "bigFont.fnt", "GJ_button_01.png", 32.0f, 1.0f)
        .intoMenuItem([this](auto) {
            this->m_editing_color = 1;
            auto popup = geode::ColorPickPopup::create(Utils::intToColor(this->m_profile_data->color1.value_or(0)));
            popup->setDelegate(this);
            popup->show();
        })
        .id("button-color1"_spr)
        .parent(main_menu);

    main_menu->setLayout(ColumnLayout::create());
    main_menu->updateLayout();

    this->m_mainLayer->addChild(main_menu);

    return true;
}

void EditBackgroundPopup::updateUI() {
    this->m_background->setStartColor(Utils::intToColor(this->m_profile_data->color1.value_or(0)));
    this->m_background->setEndColor(Utils::intToColor(this->m_profile_data->color2.value_or(0)));
}

// callback
void EditBackgroundPopup::updateColor(cocos2d::ccColor4B const& color) {
    log::info("color updated: {} {} {}", color.r, color.g, color.b);
    if (this->m_editing_color == 1) {
        this->m_profile_data->color1 = Utils::colorToInt(ccc3(color.r, color.g, color.b));
    } else {
        this->m_profile_data->color2 = Utils::colorToInt(ccc3(color.r, color.g, color.b));
    }

    this->updateUI();
}
