#pragma once

#define TEXTURE_BUTTON_ENABLED "GJ_button_01.png"
#define TEXTURE_BUTTON_DISABLED "GJ_button_04.png"
// idk clang complains about these now
// so import em ig
#include <optional>
#include <string>
#include <matjson.hpp>
#include <map>

struct ProfileData {
    int id;
    std::optional<std::string> bio; // md?
    std::optional<std::string> pronouns;

    // socials !!
    std::optional<std::string> website;
    std::optional<std::string> social_github;
    std::optional<std::string> social_bluesky;
    std::optional<std::string> social_fediverse;
    std::optional<std::string> social_discord;
    std::optional<std::string> social_matrix;
    std::optional<std::string> social_tumblr;
    std::optional<std::string> social_myspace;
    std::optional<std::string> social_facebook;

    // custom gradient colors
    std::optional<int> color1;
    std::optional<int> color2;

    auto operator<=>(const ProfileData&) const = default;
};

template <>
struct matjson::Serialize<ProfileData> {
    static geode::Result<ProfileData> fromJson(const matjson::Value& value) {
        return geode::Ok(ProfileData {
            .id = value["id"].as<int>().unwrapOr(0),
            .bio = value["bio"].asString().unwrapOr(""),
            .pronouns = value["pronouns"].asString().unwrapOr(""),
            // socials
            .website = value["website"].asString().unwrapOr(""),
            .social_github = value["social_github"].asString().unwrapOr(""),
            .social_bluesky = value["social_bluesky"].asString().unwrapOr(""),
            .social_fediverse = value["social_fediverse"].asString().unwrapOr(""),
            .social_discord = value["social_discord"].asString().unwrapOr(""),
            .social_matrix = value["social_matrix"].asString().unwrapOr(""),
            .social_tumblr = value["social_tumblr"].asString().unwrapOr(""),
            .social_myspace = value["social_myspace"].asString().unwrapOr(""),
            .social_facebook = value["social_facebook"].asString().unwrapOr(""),
            .color1 = value["color1"].as<int>().unwrapOr(0),
            .color2 = value["color2"].as<int>().unwrapOr(0),
        });
    }
    static matjson::Value toJson(const ProfileData& profile_data) {
        auto res = matjson::Value();
        res["id"] = profile_data.id;
        res["bio"] = profile_data.bio.value_or("");
        res["pronouns"] = profile_data.pronouns.value_or("");
        // socials
        res["website"] = profile_data.website.value_or("");
        res["social_github"] = profile_data.social_github.value_or("");
        res["social_bluesky"] = profile_data.social_bluesky.value_or("");
        res["social_fediverse"] = profile_data.social_fediverse.value_or("");
        res["social_discord"] = profile_data.social_discord.value_or("");
        res["social_matrix"] = profile_data.social_matrix.value_or("");
        res["social_tumblr"] = profile_data.social_tumblr.value_or("");
        res["social_myspace"] = profile_data.social_myspace.value_or("");
        res["social_facebook"] = profile_data.social_facebook.value_or("");
        res["color1"] = profile_data.color1.value_or(0);
        res["color2"] = profile_data.color2.value_or(0);

        // for some reason .value_or(nullptr) crashes, work around this by setting the value null after the fact
        if (!profile_data.bio.has_value()) res["bio"] = nullptr;
        if (!profile_data.pronouns.has_value()) res["pronouns"] = nullptr;
        if (!profile_data.website.has_value()) res["website"] = nullptr;
        if (!profile_data.social_github.has_value()) res["social_github"] = nullptr;
        if (!profile_data.social_bluesky.has_value()) res["social_bluesky"] = nullptr;
        if (!profile_data.social_fediverse.has_value()) res["social_fediverse"] = nullptr;
        if (!profile_data.social_discord.has_value()) res["social_discord"] = nullptr;
        if (!profile_data.social_matrix.has_value()) res["social_matrix"] = nullptr;
        if (!profile_data.social_tumblr.has_value()) res["social_tumblr"] = nullptr;
        if (!profile_data.social_myspace.has_value()) res["social_myspace"] = nullptr;
        if (!profile_data.social_facebook.has_value()) res["social_facebook"] = nullptr;
        if (!profile_data.color1.has_value()) res["color1"] = nullptr;
        if (!profile_data.color2.has_value()) res["color2"] = nullptr;
        return res;
    }
};
