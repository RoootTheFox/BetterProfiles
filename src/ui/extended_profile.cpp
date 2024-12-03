#include <Geode/Geode.hpp>

#include <UIBuilder.hpp>
#include "ui/extended_profile.hpp"
#include "utils.hpp"

using namespace geode::prelude;

ExtendedProfilePage* ExtendedProfilePage::create(ProfileData const& profile_data, GJUserScore* const& user_score) {
    auto ret = new ExtendedProfilePage();
    ret->m_profile_data = profile_data;
    ret->m_user_score = user_score;
    if (profile_data.id == 0) ret->m_profile_data.id = user_score->m_accountID;
    if (ret && ret->initAnchored(441.f, 292.f, profile_data, user_score)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool ExtendedProfilePage::setup(ProfileData const& profile_data, GJUserScore* const& user_score) {
    auto win_size = CCDirector::sharedDirector()->getWinSize();
    auto game_manager = GameManager::sharedState();

    auto player_col1 = game_manager->colorForIdx(user_score->m_color1);
    auto player_col2 = game_manager->colorForIdx(user_score->m_color2);

    auto col1 = profile_data.color1.has_value() ? Utils::intToColor(profile_data.color1.value_or(0)) : player_col1;
    auto col2 = profile_data.color2.has_value() ? Utils::intToColor(profile_data.color2.value_or(0)) : player_col2;

    auto bg_sprite = this->m_mainLayer->getChildByType<CCScale9Sprite>(0);

    auto popup_content_size = CCSize(0.f, 0.f);
    if (bg_sprite) {
        popup_content_size = bg_sprite->getContentSize();
        bg_sprite->setVisible(false);

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

    Build<MDTextArea>::create("", ccp(280.f, 150.f))
        .pos(win_size / 2)
        .store(this->m_bio_area)
        .parent(this->m_mainLayer);

    this->setTitle(fmt::format("{}'s Profile", user_score->m_userName));

    auto spider = Build<SimplePlayer>::create(0)
        .playerFrame(user_score->m_playerSpider, IconType::Spider)
        .color(player_col1)
        .secondColor(player_col2)
        .pos({win_size.width / 4 + 32.f, 32.f})
        .parent(this->m_mainLayer)
        .collect();

    if (user_score->m_glowEnabled) {
        spider->setGlowOutline(game_manager->colorForIdx(user_score->m_color3));
    }

    auto menu = Build<CCMenu>::create()
        .pos(win_size / 2)
        .parent(this->m_mainLayer)
        .collect();

    Build<geode::CircleButtonSprite>::create(CCSprite::createWithSpriteFrameName("pencil.png"_spr), CircleBaseColor::Green, CircleBaseSize::MediumAlt)
        .scale(0.65f)
        .intoMenuItem([this](auto) {
            log::info("edit button clicked");
            auto edit_page = EditPage::create(this->m_profile_data);
            edit_page->setCallback(this->m_callback);
            edit_page->show();
        })
            .id("edit-button"_spr)
            .visible(false)
            .store(this->m_edit_button)
            .parent(menu)
            .pos((popup_content_size / 2) - 24.f);

    this->updateUI();

    return true;
}

void ExtendedProfilePage::updateUI() {
    auto player_col1 = GameManager::sharedState()->colorForIdx(this->m_user_score->m_color1);
    auto player_col2 = GameManager::sharedState()->colorForIdx(this->m_user_score->m_color2);

    auto col1 = this->m_profile_data.color1.has_value() ? Utils::intToColor(this->m_profile_data.color1.value_or(0)) : player_col1;
    auto col2 = this->m_profile_data.color2.has_value() ? Utils::intToColor(this->m_profile_data.color2.value_or(0)) : player_col2;
    this->m_background->setStartColor(col1);
    this->m_background->setEndColor(col2);

    this->m_bio_area->setString(this->m_profile_data.bio.value_or("this player <cr>doesn't</c> have a bio set!").c_str());
}

void ExtendedProfilePage::setCallback(std::function<void(ProfileData &)> callback) {
    this->m_callback = [this, callback](ProfileData &data) {
        this->m_profile_data = data;
        this->updateUI();
        callback(data);
    };
    if (this->m_edit_button) {
        this->m_edit_button->setVisible(true);
    }
}
