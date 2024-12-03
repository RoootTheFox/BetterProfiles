#include <Geode/Geode.hpp>
#include <UIBuilder.hpp>
#include <string>
#include <vector>

#include "ui/edit_pronouns.hpp"

using namespace geode::prelude;

EditPronounsPopup* EditPronounsPopup::create(ProfileData* const& profile_data) {
    auto ret = new EditPronounsPopup();
    if (ret && ret->initAnchored(450.f, 280.f, profile_data)) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool EditPronounsPopup::setup(ProfileData* const& profile_data) {
    this->m_profile_data = profile_data;
    log::info("current pronouns: {}", profile_data->pronouns);
    auto win_size = CCDirector::sharedDirector()->getWinSize();

    this->setTitle("Change Pronouns");

    Build<TextInput>::create(128.f, "they/them", "bigFont.fnt")
        .id("pronouns-input"_spr)
        .pos(win_size.width / 2, 70.f)
        .parent(m_mainLayer)
        .store(m_pronouns_input);
    m_pronouns_input->setFilter("abcdefghijklmnopqrstuvwxyz/");

    m_pronouns_input->setString(this->m_profile_data->pronouns.value_or("").c_str());
    m_pronouns_input->setCallback([this](const std::string& text) {
        this->m_profile_data->pronouns = text;
        this->updateUI();
    });

    Build<CCMenu>::create()
        .id("pronoun-sets-menu"_spr)
        .layout(RowLayout::create()->setGap(18.f))
        .pos(win_size.width / 2, win_size.height / 2 + 16.f)
        .parent(m_mainLayer)
        .child(this->createPronounSet(1))
        .child(this->createPronounSet(2))
        .child(this->createPronounSet(3))
        .updateLayout()
        .store(m_pronoun_sets_menu);

    this->parsePronouns(profile_data->pronouns.value_or(""));
    this->updateUI();

    Build<CCLabelBMFont>::create("Note: This is an accessibility feature - please use it responsively.", "goldFont.fnt")
        .id("custom-notice"_spr)
        .scale(0.5f)
        .pos(win_size.width / 2, 45.f)
        .parent(m_mainLayer)
        .intoNewSibling(CCLabelBMFont::create("(abuse will result in a ban)", "goldFont.fnt"))
            .scale(0.5f)
            .pos(win_size.width / 2, 35.f);

    return true;
}

std::vector<std::string> EditPronounsPopup::parsePronouns(std::string const& pronouns_const) {
    std::string pronouns = pronouns_const;
    std::vector<std::string> sets;

    std::string tmp;
    std::stringstream pronouns_ss(pronouns);
    while (getline(pronouns_ss, tmp, '/')) {
        // ignore if it's a 2nd pronoun
        if (!(tmp == "them" || tmp == "its" || tmp == "her" || tmp == "him")) {
            sets.push_back(tmp);
        }
    }

    return sets;
}

std::string EditPronounsPopup::getModifiedPronouns(std::vector<std::string> pronouns, int set, std::string pronoun) {
    if (pronouns.size() < set) {
        log::debug("resizing pronouns to {}", set);
        pronouns.resize(set);
    }

    if (pronouns.at(set - 1) == pronoun) {
        if (set == 1 && pronouns.size() == 1) {
            return ""; // no pronouns (prevent freeze)
        }
        pronouns.erase(pronouns.begin() + (set - 1));
    } else {
        if (!(set > 1 && (pronouns.at(set - 2) == pronoun || set > 2 && pronouns.at(set - 3) == pronoun))) {
            pronouns.at(set - 1) = pronoun;
        } else {
            pronouns.erase(pronouns.begin() + (set - 1));
        }
    }

    int pronoun_set_count = pronouns.size();
    std::string new_pronouns = "";
    for (auto const& p : pronouns) {
        if (p != "") {
            new_pronouns += p + "/";
        } else {
            pronoun_set_count--;
        }
    }
    new_pronouns.pop_back();

    if (pronoun_set_count == 1) {
        std::string second_pronoun;
        if (new_pronouns == "they") {
            second_pronoun = "them";
        } else if (new_pronouns == "it") {
            second_pronoun = "its";
        } else if (new_pronouns == "she") {
            second_pronoun = "her";
        } else if (new_pronouns == "he") {
            second_pronoun = "him";
        }
        new_pronouns += "/" + second_pronoun;
    }

    return new_pronouns;
}

void EditPronounsPopup::updateUI() {
    auto pronouns = this->parsePronouns(this->m_profile_data->pronouns.value_or(""));

    for (int i = 0; i < 3; i++) {
        auto set = i + 1;

        std::string pronoun = "";
        if (pronouns.size() < set) {
            //log::info("no pronouns for set {}", set);
        } else {
            pronoun = pronouns.at(i);
        }

        //log::info("pronoun {} in set {}", pronoun, set);

        auto set_menu = typeinfo_cast<CCMenu*>(m_pronoun_sets_menu->getChildByID(fmt::format("set-{}", set)));
        if (!set_menu) {
            log::error("failed to get pronoun set menu for set {}", set);
            return;
        }

        // loop over set_menu children
        for (int j = 0; j < set_menu->getChildren()->count(); j++) {
            if (auto button = set_menu->getChildByType<CCMenuItemSpriteExtra>(j)) {
                if (auto button_sprite = button->getChildByType<ButtonSprite>(0)) {
                    button_sprite->updateBGImage(TEXTURE_BUTTON_DISABLED);
                }
            }
        }

        if (auto button = typeinfo_cast<CCMenuItemSpriteExtra*>(set_menu->getChildByID(pronoun))) {
            if (auto button_sprite = button->getChildByType<ButtonSprite>(0)) {
                button_sprite->updateBGImage(TEXTURE_BUTTON_ENABLED);
            }
        }
    }

    // update pronouns label
    this->m_pronouns_input->setString(this->m_profile_data->pronouns.value_or("").c_str());
}

CCMenu* EditPronounsPopup::createPronounSet(int set) {
    auto menu = Build<CCMenu>::create()
        .layout(ColumnLayout::create())
        .tag(set)
        .id(fmt::format("set-{}", set))
        .collect();

    Build<ButtonSprite>::create("they/them", 40, true, "bigFont.fnt", TEXTURE_BUTTON_ENABLED, 32.0f, 1.0f)
        .intoMenuItem([this, set](auto) {
            this->onPronounButtonClicked(set, "they");
        }).id("they").parent(menu);

    Build<ButtonSprite>::create("it/its", 40, true, "bigFont.fnt", TEXTURE_BUTTON_ENABLED, 32.0f, 1.0f)
        .intoMenuItem([this, set](auto) {
            this->onPronounButtonClicked(set, "it");
        }).id("it").parent(menu);

    Build<ButtonSprite>::create("she/her", 40, true, "bigFont.fnt", TEXTURE_BUTTON_ENABLED, 32.0f, 1.0f)
        .intoMenuItem([this, set](auto) {
            this->onPronounButtonClicked(set, "she");
        }).id("she").parent(menu);

    Build<ButtonSprite>::create("he/him", 40, true, "bigFont.fnt", TEXTURE_BUTTON_ENABLED, 32.0f, 1.0f)
        .intoMenuItem([this, set](auto) {
            this->onPronounButtonClicked(set, "he");
        }).id("he").parent(menu);

    Build<CCLabelBMFont>::create(fmt::format("set {}", set).c_str(), "goldFont.fnt")
        .scale(0.5f)
        .parent(menu);

    menu->updateLayout();

    return menu;
}

// this should *ONLY* be called for buttons inside of a pronoun set ccmenu
void EditPronounsPopup::onPronounButtonClicked(int set, std::string pronoun) {
    std::vector<std::string> pronouns = this->parsePronouns(this->m_profile_data->pronouns.value_or(""));

    //std::vector<std::string> pronouns = this->parsePronouns("she/her");
    if (pronouns.size() + 1 < set && (pronouns.size() != 0 && set != 1)) {
        Notification::create(fmt::format("Select set {} before trying to change set {}", set - 1, set), NotificationIcon::Error)->show();
        return;
    }

    log::info("current pronouns: {} ({})", pronouns, pronouns.size());

    this->m_profile_data->pronouns = this->getModifiedPronouns(pronouns, set, pronoun);
    this->updateUI();
}
