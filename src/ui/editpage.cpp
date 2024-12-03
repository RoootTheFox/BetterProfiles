#include <Geode/Geode.hpp>

#include "backend.hpp"
#include "ui/editpage.hpp"
#include "ui/edit_pronouns.hpp"
#include "ui/edit_bio.hpp"
#include "ui/edit_background.hpp"
#include <UIBuilder.hpp>

using namespace geode::prelude;

// this is to prevent crashes
static EditPage* current_edit_page = nullptr;
EditPage::~EditPage() {
    current_edit_page = nullptr;
}

EditPage* EditPage::create(ProfileData const& profile_data) {
    auto ret = new EditPage();
    if (ret && ret->initAnchored(450.f, 280.f, profile_data)) {
        ret->m_profile_data = profile_data;
        ret->m_original_data = profile_data;
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool EditPage::setup(ProfileData const& profile_data) {
    current_edit_page = this;
    auto win_size = CCDirector::sharedDirector()->getWinSize();

    this->setTitle("Customize Profile");

    m_buttonMenu->ignoreAnchorPointForPosition(true);

    // if the user isn't logged in, show a login button
    if (Mod::get()->getSavedValue<std::string>("token", "").empty()) {
        log::info("not logged in!");

        Build<ButtonSprite>::create("Login", 64, true, "bigFont.fnt", "GJ_button_01.png", 32.0f, 1.0f)
            .store(m_login_sprite)
            .intoMenuItem([this](auto) {
                this->onLogin();
            })
                .store(m_login_button)
                .id("login-button"_spr)
                .pos(0.f, -96.f)
                .parent(m_buttonMenu);

        // login prompt
        Build<CCLabelBMFont>::create("You aren't logged in!", "bigFont.fnt")
            .id("login-prompt-1"_spr)
            .scale(0.75f)
            .parentAtPos(m_mainLayer, Anchor::Center)
            .store(m_login_prompt_1)
            .intoParent<CCLabelBMFont>().create("Click the button below to log in.", "bigFont.fnt")
                .id("login-prompt-2"_spr)
                .scale(0.65f)
                .parentAtPos(m_mainLayer, geode::Anchor::Center)
                .store(m_login_prompt_2);

        m_login_prompt_1->setPositionY(m_login_prompt_1->getPositionY() + 22.f);

        return true;
    }

    log::info("token present, initializing UI");
    this->setupLoggedIn();

    return true;
}

void EditPage::setupLoggedIn() {
    // hide all login-related nodes if they're present
    if(this->m_login_prompt_1) this->m_login_prompt_1->setVisible(false);
    if(this->m_login_prompt_2) this->m_login_prompt_2->setVisible(false);
    if(this->m_login_button) this->m_login_button->setVisible(false);
    if(this->m_login_loading_circle) this->m_login_loading_circle->setVisible(false);

    auto win_size = CCDirector::sharedDirector()->getWinSize();

    // actual UI setup below
    auto main_menu = Build<CCMenu>::create()
        .layout(ColumnLayout::create())
        .parentAtPos(m_mainLayer, Anchor::Center)
        .collect();

    Build<CCLabelBMFont>::create("meow", "bigFont.fnt")
        .parent(main_menu);

    // create update buttons for each field in the profile data

    // pronouns
    Build<ButtonSprite>::create("Change Pronouns", 122, true, "bigFont.fnt", "GJ_button_01.png", 32.0f, 1.0f)
        .intoMenuItem([this](auto) {
            this->onEditPronouns();
        })
        .id("pronouns-button"_spr)
        .parent(main_menu);

    // bio
    Build<ButtonSprite>::create("Change Bio", 80, true, "bigFont.fnt", "GJ_button_01.png", 32.0f, 1.0f)
        .intoMenuItem([](auto) {
            EditBioPopup::create(&(current_edit_page->m_profile_data))->show();
        })
        .id("bio-button"_spr)
        .parent(main_menu);

    // socials
    Build<ButtonSprite>::create("Change Socials", 122, true, "bigFont.fnt", "GJ_button_01.png", 32.0f, 1.0f)
        .intoMenuItem([](auto) {})
        .id("socials-button"_spr)
        .parent(main_menu);

    // background
    Build<ButtonSprite>::create("Change Background", 122, true, "bigFont.fnt", "GJ_button_01.png", 32.0f, 1.0f)
        .intoMenuItem([](auto) {
            EditBackgroundPopup::create(&(current_edit_page->m_profile_data))->show();
        })
        .id("background-button"_spr)
        .parent(main_menu);

    main_menu->updateLayout();

    // save button
    Build<ButtonSprite>::create("Save", 64, true, "bigFont.fnt", "GJ_button_01.png", 32.0f, 1.0f)
        .intoMenuItem([this](auto) {
            this->onSave();
        })
        .pos(0.f, -96.f)
        .id("save-button"_spr)
        .visible(false)
        .parent(m_buttonMenu)
        .store(m_save_button);

    // logout button
    Build<ButtonSprite>::create("Logout", 64, true, "bigFont.fnt", "GJ_button_06.png", 32.0f, 1.0f)
        .intoMenuItem([this](auto) {
            Mod::get()->setSavedValue<std::string>("dashauth_token_rooot.betterprofiles", "");
            Mod::get()->setSavedValue<std::string>("token", "");
            this->removeFromParent();
        })
        .pos(180.f, 118.f)
        .id("logout-button"_spr)
        .parent(m_buttonMenu);
}

void EditPage::onLogin() {
    // user clicked the login button

    // hide login prompt, make button unclickable and grayed out, show loading circle

    if(m_login_prompt_1) m_login_prompt_1->setVisible(false);
    if(m_login_prompt_2) m_login_prompt_2->setVisible(false);
    if(m_login_button) {
        m_login_button->setEnabled(false);

        m_login_sprite->setString("Loading...");
        m_login_sprite->updateBGImage("GJ_button_04.png");
        m_login_sprite->m_label->setColor(ccc3(175, 175, 175));
    }

    if (this->m_login_loading_circle != nullptr) {
        m_login_loading_circle->fadeAndRemove();
        m_login_loading_circle->removeFromParentAndCleanup(true);
        m_login_loading_circle = nullptr;
    }

    Build<LoadingCircle>::create()
        .pos(0.f, 0.f)
        .id("loading_circle"_spr)
        .parent(m_mainLayer)
        .store(m_login_loading_circle);
    m_login_loading_circle->show();

    dashauth::DashAuthRequest().getToken(Mod::get(), fmt::format("{}/api/v1", BACKEND_PREFIX))->except([](std::string const& error) {
        log::info("login failed: {}", error);

        // make sure that we can fuck with the ui
        if (current_edit_page != nullptr) {
            // reset all login related nodes
            if(current_edit_page->m_login_prompt_1) current_edit_page->m_login_prompt_1->setVisible(true);
            if(current_edit_page->m_login_prompt_2) current_edit_page->m_login_prompt_2->setVisible(true);
            log::info("set login prompt visible");

            if(current_edit_page->m_login_button) {
                current_edit_page->m_login_button->setEnabled(true);

                if (auto button_sprite = current_edit_page->m_login_button->getChildByType<ButtonSprite>(0)) {
                    button_sprite->setString("Login");
                    button_sprite->updateBGImage("GJ_button_01.png");
                    button_sprite->m_label->setColor(ccWHITE);
                }
            }

            if(current_edit_page->m_login_loading_circle) {
                current_edit_page->m_login_loading_circle->fadeAndRemove();
                current_edit_page->m_login_loading_circle->removeFromParentAndCleanup(true);
                current_edit_page->m_login_loading_circle->setVisible(false); // ugly hack, nobody will notice
                current_edit_page->m_login_loading_circle = nullptr;
            }
        }

        log::info("failed to get token :c");
        FLAlertLayer::create("Authentication Error", "failed to get token :c", "OK")->show();
    })->then([](std::string const& token) {
        log::info("got token!! {} :3", token);
        Mod::get()->setSavedValue("token", token);

        if (current_edit_page != nullptr) {
            current_edit_page->setupLoggedIn();
            if(current_edit_page->m_login_loading_circle) {
                current_edit_page->m_login_loading_circle->fadeAndRemove();
                current_edit_page->m_login_loading_circle->removeFromParentAndCleanup(true);
                current_edit_page->m_login_loading_circle->setVisible(false); // dumb hack, nobody will notice
                current_edit_page->m_login_loading_circle = nullptr;
            }
        }
    });
}

void EditPage::onEditPronouns() {
    // user clicked the pronouns button
    log::info("edit pronouns");

    EditPronounsPopup::create(&(this->m_profile_data))->show();
}

void EditPage::onSave() {
    log::info("saving profile data");

    // loading circle !!
    if (this->m_save_loading_circle) {
        m_save_loading_circle->fadeAndRemove();
        m_save_loading_circle->removeFromParentAndCleanup(true);
        m_save_loading_circle = nullptr;
    }

    Build<LoadingCircle>::create()
        .pos(0.f, 0.f)
        .id("loading_circle"_spr)
        .parent(m_mainLayer)
        .store(m_save_loading_circle);

    m_save_loading_circle->show();

    m_web_listener.bind([this] (web::WebTask::Event* e) {
        if (web::WebResponse* res = e->getValue()) {
            auto response = res->json();
            if (response.isErr()) {
                auto error_response = response.isErr() ? response.err() : "unknown error";
                log::info("failed to save profile data: {}", error_response);

                auto error = error_response.has_value() ? error_response.value() : "unknown error wtf";

                std::string error_str = "";
                auto json_opt = matjson::parse(error);

                if (json_opt.isErr()) {
                    log::error("failed to parse json: {}\nServer returned {}", error_str, error);
                    std::string display_error = error;
                    if (error.length() > 100) display_error = "(error too long, check logs for details)";
                    error_str = "Server returned invalid response:\n<cr>" + display_error + "</c>\n" + "Server might be down, check your internet connection and try again later.";
                } else {
                    auto json = json_opt.unwrap();
                    error_str = json.contains("message") ? json["message"].asString().unwrapOr("") : "Server returned invalid JSON response";
                }

                FLAlertLayer::create("Failed to save profile", error_str, "OK")->show();
                if (current_edit_page == nullptr) return;
                current_edit_page->m_save_loading_circle->setVisible(false); // i hate loading circles
                current_edit_page->m_save_loading_circle->fadeAndRemove();
                current_edit_page->m_save_loading_circle->removeFromParentAndCleanup(true);
                current_edit_page->m_save_loading_circle = nullptr;
                return;
            }

            auto json = response.unwrap();
            log::info("saved profile data !!");

            if (current_edit_page == nullptr) return;
            if(current_edit_page->m_callback != nullptr) current_edit_page->m_callback(current_edit_page->m_profile_data);
            current_edit_page->m_original_data = current_edit_page->m_profile_data;
            current_edit_page->m_save_loading_circle->setVisible(false);
            current_edit_page->m_save_loading_circle->fadeAndRemove();
            current_edit_page->m_save_loading_circle->removeFromParentAndCleanup(true);
            current_edit_page->m_save_loading_circle = nullptr;
            current_edit_page->removeFromParent();
        } else if (web::WebProgress* p = e->getProgress()) {
            // meow :3
        } else if (e->isCancelled()) {
            log::info("The request was cancelled... So sad :(");
        }
    });

    auto req = web::WebRequest();
    req.bodyJSON(this->m_profile_data);
    req.header("Authorization", fmt::format("Bearer {}", Mod::get()->getSavedValue<std::string>("token", "")));
    m_web_listener.setFilter(req.post(fmt::format("{}/api/v1/profiles/{}", BACKEND_PREFIX, this->m_profile_data.id)));
}

void EditPage::keyDown(cocos2d::enumKeyCodes key) {
    if (key == cocos2d::enumKeyCodes::KEY_Escape && (this->m_profile_data != this->m_original_data)) {
        geode::createQuickPopup(
            "Warning",
            "You have <cr>unsaved changes!</c> Do you want to <cr>discard</c> them?",
            "Go back", "Discard",
            [this, key](auto, bool discard) {
                if (discard) {
                    geode::Popup<ProfileData const&>::keyDown(key);
                }
            }
        );
    } else {
        geode::Popup<ProfileData const&>::keyDown(key);
    }
}

// draw is called every frame, so we can "hook" it to determine when to show the save button
void EditPage::draw() {
    geode::Popup<ProfileData const&>::draw();
    if (this->m_save_button == nullptr) return;

    if (this->m_profile_data != this->m_original_data) {
        this->m_save_button->setVisible(true);
    } else {
        this->m_save_button->setVisible(false);
    }
}

void EditPage::setCallback(std::function<void(ProfileData &)> callback) {
    this->m_callback = callback;
}
